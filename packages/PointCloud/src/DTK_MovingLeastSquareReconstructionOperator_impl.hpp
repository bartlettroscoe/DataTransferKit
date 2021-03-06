//---------------------------------------------------------------------------//
/*
  Copyright (c) 2014, Stuart R. Slattery
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  *: Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  *: Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  *: Neither the name of the Oak Ridge National Laboratory nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//---------------------------------------------------------------------------//
/*!
 * \file   DTK_MovingLeastSquareReconstructionOperator_impl.hpp
 * \author Stuart R. Slattery
 * \brief  Moving least square interpolator.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_MOVINGLEASTSQUARERECONSTRUCTIONOPERATOR_IMPL_HPP
#define DTK_MOVINGLEASTSQUARERECONSTRUCTIONOPERATOR_IMPL_HPP

#include "DTK_DBC.hpp"
#include "DTK_LocalMLSProblem.hpp"
#include "DTK_CenterDistributor.hpp"
#include "DTK_SplineInterpolationPairing.hpp"

#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_Ptr.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_ParameterList.hpp>

#include <Tpetra_MultiVector.hpp>
#include <Tpetra_CrsMatrix.hpp>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
// Constructor.
template<class Scalar,class Basis,int DIM>
MovingLeastSquareReconstructionOperator<Scalar,Basis,DIM>::MovingLeastSquareReconstructionOperator( const double radius )
    : d_radius( radius )
{ /* ... */ }

//---------------------------------------------------------------------------//
// Destructor.
template<class Scalar,class Basis,int DIM>
MovingLeastSquareReconstructionOperator<Scalar,Basis,DIM>::~MovingLeastSquareReconstructionOperator()
{ /* ... */ }

//---------------------------------------------------------------------------//
// Setup the map operator.
template<class Scalar,class Basis,int DIM>
void MovingLeastSquareReconstructionOperator<Scalar,Basis,DIM>::setup(
    const Teuchos::RCP<const typename Base::TpetraMap>& domain_map,
    const Teuchos::RCP<FunctionSpace>& domain_space,
    const Teuchos::RCP<const typename Base::TpetraMap>& range_map,
    const Teuchos::RCP<FunctionSpace>& range_space,
    const Teuchos::RCP<Teuchos::ParameterList>& parameters )
{
    DTK_REQUIRE( Teuchos::nonnull(domain_map) );
    DTK_REQUIRE( Teuchos::nonnull(domain_space) );
    DTK_REQUIRE( Teuchos::nonnull(range_map) );
    DTK_REQUIRE( Teuchos::nonnull(range_space) );
    DTK_REQUIRE( Teuchos::nonnull(parameters) );

    // Get the parallel communicator.
    Teuchos::RCP<const Teuchos::Comm<int> > comm = domain_map->getComm();

    // Determine if we have range and domain data on this process.
    bool nonnull_domain = Teuchos::nonnull( domain_space->entitySet() );
    bool nonnull_range = Teuchos::nonnull( range_space->entitySet() );

    // Make sure we are applying the map to nodes.
    DTK_REQUIRE( domain_space->entitySelector()->entityType() ==
		 ENTITY_TYPE_NODE );
    DTK_REQUIRE( range_space->entitySelector()->entityType() ==
		 ENTITY_TYPE_NODE );

    // Extract the DOF maps.
    this->b_domain_map = domain_map;
    this->b_range_map = range_map;

    // Extract the source centers and their ids.
    EntityIterator domain_iterator;
    if ( nonnull_domain )
    {
	domain_iterator = domain_space->entitySet()->entityIterator( 
	    domain_space->entitySelector()->entityType(),
	    domain_space->entitySelector()->selectFunction() );
    }
    int local_num_src = domain_iterator.size();
    Teuchos::ArrayRCP<double> source_centers( DIM*local_num_src);
    Teuchos::ArrayRCP<GO> source_gids( local_num_src );
    EntityIterator domain_begin = domain_iterator.begin();
    EntityIterator domain_end = domain_iterator.end();
    int entity_counter = 0;
    for ( EntityIterator domain_entity = domain_begin;
	  domain_entity != domain_end;
	  ++domain_entity, ++entity_counter )
    {
	source_gids[entity_counter] = domain_entity->id();
	domain_space->localMap()->centroid(
	    *domain_entity, source_centers(DIM*entity_counter,DIM) );
    }

    // Extract the target centers and their ids.
    EntityIterator range_iterator;
    if ( nonnull_range )
    {
	range_iterator = range_space->entitySet()->entityIterator( 
	    range_space->entitySelector()->entityType(),
	    range_space->entitySelector()->selectFunction() );
    } 
    int local_num_tgt = range_iterator.size();
    Teuchos::ArrayRCP<double> target_centers( DIM*local_num_tgt );
    Teuchos::ArrayRCP<GO> target_gids( local_num_tgt );
    EntityIterator range_begin = range_iterator.begin();
    EntityIterator range_end = range_iterator.end();
    entity_counter = 0;
    for ( EntityIterator range_entity = range_begin;
	  range_entity != range_end;
	  ++range_entity, ++entity_counter )
    {
	target_gids[entity_counter] = range_entity->id();
	range_space->localMap()->centroid(
	    *range_entity, target_centers(DIM*entity_counter,DIM) );
    }

    // Build the basis.
    Teuchos::RCP<Basis> basis = BP::create( d_radius );

    // Gather the source centers that are within a radius of the target
    // centers on this proc.
    Teuchos::Array<double> dist_sources;
    CenterDistributor<DIM> distributor( 
	comm, source_centers(), target_centers(), d_radius, dist_sources );

    // Gather the global ids of the source centers that are within a radius of
    // the target centers on this proc.
    Teuchos::Array<GO> dist_source_gids( distributor.getNumImports() );
    Teuchos::ArrayView<const GO> source_gids_view = source_gids();
    distributor.distribute( source_gids_view, dist_source_gids() );

    // Build the source/target pairings.
    SplineInterpolationPairing<DIM> pairings( 
	dist_sources, target_centers(), d_radius );

    // Build the interpolation matrix.
    Teuchos::ArrayRCP<std::size_t> children_per_parent =
	pairings.childrenPerParent();
    std::size_t max_entries_per_row = *std::max_element( 
	children_per_parent.begin(), children_per_parent.end() );
    Teuchos::RCP<Tpetra::CrsMatrix<Scalar,int,GO> > H = 
	Teuchos::rcp( new Tpetra::CrsMatrix<Scalar,int,GO>( 
			  this->b_range_map,
			  max_entries_per_row) );
    Teuchos::ArrayView<const Scalar> target_view;
    Teuchos::Array<GO> indices( max_entries_per_row );
    Teuchos::ArrayView<const Scalar> values;
    Teuchos::ArrayView<const unsigned> pair_gids;
    int nn = 0;
    for ( int i = 0; i < local_num_tgt; ++i )
    {
	// If there is no support for this target center then do not build a
	// local basis.
	if ( 0 < pairings.childCenterIds(i).size() )
	{
	    // Get a view of this target center.
	    target_view = target_centers(i*DIM,DIM);

	    // Build the local interpolation problem. 
	    LocalMLSProblem<Basis,DIM> local_problem(
		target_view, pairings.childCenterIds(i),
		dist_sources, *basis );

	    // Get MLS shape function values for this target point.
	    values = local_problem.shapeFunction();
	    nn = values.size();

	    // Populate the interpolation matrix row.
	    pair_gids = pairings.childCenterIds(i);
	    for ( int j = 0; j < nn; ++j )
	    {
		indices[j] = dist_source_gids[ pair_gids[j] ];
	    }
	    H->insertGlobalValues( target_gids[i], indices(0,nn), values );
	}
    }
    H->fillComplete( this->b_domain_map, this->b_range_map );
    DTK_ENSURE( H->isFillComplete() );
    
    // Wrap the interpolation matrix in a Thyra wrapper.
    Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > thyra_range_vector_space =
    	Thyra::createVectorSpace<Scalar>( H->getRangeMap() );
    Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > thyra_domain_vector_space =
    	Thyra::createVectorSpace<Scalar>( H->getDomainMap() );
    Teuchos::RCP<Thyra::TpetraLinearOp<Scalar,LO,GO> > thyra_H =
    	Teuchos::rcp( new Thyra::TpetraLinearOp<Scalar,LO,GO>() );
    thyra_H->initialize( thyra_range_vector_space, thyra_domain_vector_space, H );

    // Set the coupling matrix with the base class.
    this->b_coupling_matrix = thyra_H;
    DTK_ENSURE( Teuchos::nonnull(this->b_coupling_matrix) );
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//

#endif // end DTK_MOVINGLEASTSQUARERECONSTRUCTIONOPERATOR_IMPL_HPP

//---------------------------------------------------------------------------//
// end DTK_MovingLeastSquareReconstructionOperator_impl.hpp
//---------------------------------------------------------------------------//

