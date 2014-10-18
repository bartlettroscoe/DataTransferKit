//---------------------------------------------------------------------------//
/*
  Copyright (c) 2012, Stuart R. Slattery
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  *: Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  *: Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  *: Neither the name of the University of Wisconsin - Madison nor the
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
 * \file DTK_GeometryRCB_def.hpp
 * \author Stuart R. Slattery
 * \brief Wrapper definition for Zoltan recursive coordinate bisectioning.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_GEOMETRYRCB_DEF_HPP
#define DTK_GEOMETRYRCB_DEF_HPP

#include <numeric>
#include <algorithm>

#include "DTK_DBC.hpp"
#include "DTK_CommIndexer.hpp"
#include "DTK_GeometryRCB.hpp"

#include <mpi.h>

#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_OpaqueWrapper.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_Tuple.hpp>
#include <Teuchos_as.hpp>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
 * \brief Constructor.
 *
 * \param comm The communicator over which to build the GeometryRCB partitioning.
 *
 * \param geometry_manager The geometry to be partitioned with GeometryRCB. A
 * null RCP is valid here as the geometry may or may not exist on all of the
 * processes we want to repartition it to.
 *
 * \param dimension The dimension of the GeometryRCB space.
 */
GeometryRCB::GeometryRCB(
    const Teuchos::RCP<const Teuchos::Comm<int> >& comm, 
    const Teuchos::RCP<GeometryManager>& geometry_manager, 
    const int dimension )
    : d_comm( comm )
    , d_geometry_manager( geometry_manager )
    , d_dimension( dimension )
{
    // Get the raw MPI communicator.
    Teuchos::RCP< const Teuchos::MpiComm<int> > mpi_comm = 
	Teuchos::rcp_dynamic_cast< const Teuchos::MpiComm<int> >( comm );
    Teuchos::RCP< const Teuchos::OpaqueWrapper<MPI_Comm> > opaque_comm = 
	mpi_comm->getRawMpiComm();
    MPI_Comm raw_comm = (*opaque_comm)();

    // Create the Zoltan object.
    d_zz = Zoltan_Create( raw_comm );

    // General parameters.
    Zoltan_Set_Param( d_zz, "DEBUG_LEVEL", "0" );
    Zoltan_Set_Param( d_zz, "LB_METHOD", "RCB" );
    Zoltan_Set_Param( d_zz, "NUM_GID_ENTRIES", "1" ); 
    Zoltan_Set_Param( d_zz, "NUM_LID_ENTRIES", "1" );
    Zoltan_Set_Param( d_zz, "DEBUG_PROCESSOR", "0" );
    Zoltan_Set_Param( d_zz, "OBJ_WEIGHT_DIM", "0" );
    Zoltan_Set_Param( d_zz, "EDGE_WEIGHT_DIM", "0" );
    Zoltan_Set_Param( d_zz, "RETURN_LISTS", "ALL" );

    // GeometryRCB parameters.
    Zoltan_Set_Param( d_zz, "RCB_OUTPUT_LEVEL", "0" );
    Zoltan_Set_Param( d_zz, "RCB_RECTILINEAR_BLOCKS", "1" );
    Zoltan_Set_Param( d_zz, "KEEP_CUTS", "1" );
    Zoltan_Set_Param( d_zz, "AVERAGE_CUTS", "1" );
    Zoltan_Set_Param( d_zz, "RCB_LOCK_DIRECTIONS", "1" );
    Zoltan_Set_Param( d_zz, "RCB_SET_DIRECTIONS", "1" );

    // Register static functions.
    Zoltan_Set_Num_Obj_Fn( d_zz, getNumberOfObjects, &d_geometry_manager );
    Zoltan_Set_Obj_List_Fn( d_zz, getObjectList, &d_geometry_manager );
    Zoltan_Set_Num_Geom_Fn( d_zz, getNumGeometry, &d_dimension );
    Zoltan_Set_Geom_Multi_Fn( d_zz, getGeometryList, &d_geometry_manager );
}

//---------------------------------------------------------------------------//
/*!
 * \brief Destructor. Zoltan memory deallocation happens here and only here.
 */
GeometryRCB::~GeometryRCB()
{
    Zoltan_LB_Free_Part( &d_import_global_ids, &d_import_local_ids, 
			 &d_import_procs, &d_import_to_part );
    Zoltan_LB_Free_Part( &d_export_global_ids, &d_export_local_ids, 
			 &d_export_procs, &d_export_to_part );
    Zoltan_Destroy( &d_zz );
}

//---------------------------------------------------------------------------//
/*!
 * \brief Compute GeometryRCB partitioning of the geometry.
 */
void GeometryRCB::partition( const BoundingBox& local_box )
{
    // Partition the problem.
    DTK_CHECK_ERROR_CODE(
	Zoltan_LB_Partition( d_zz, 
			     &d_changes,  
			     &d_num_gid_entries,
			     &d_num_lid_entries,
			     &d_num_import,    
			     &d_import_global_ids,
			     &d_import_local_ids, 
			     &d_import_procs,    
			     &d_import_to_part,   
			     &d_num_export,      
			     &d_export_global_ids,
			     &d_export_local_ids, 
			     &d_export_procs,    
			     &d_export_to_part )
	);

    // Get all of the bounding boxes for future searching.
    Teuchos::Tuple<double,6> bounds;
    if ( 1 < d_comm->getSize() )
    {
	int dim = 0;
	for ( int rank = 0; rank < d_comm->getSize(); ++rank )
	{
	    DTK_CHECK_ERROR_CODE(
		Zoltan_RCB_Box( d_zz, rank, &dim,
				&bounds[0], &bounds[1], &bounds[2],
				&bounds[3], &bounds[4], &bounds[5] )
		); 
	    DTK_CHECK( dim == d_dimension );

	    // If there was no geometry in the local domain for a given rank
	    // then Zoltan will give us a bogus bounding box. Check for this.
	    if ( (bounds[3] >= bounds[0]) &&
		 (bounds[4] >= bounds[1]) &&
		 (bounds[5] >= bounds[2]) )
	    {
		// If the bounding box is valid, see if the partition domain
		// intersects the local domain.
		BoundingBox partition_box( bounds );
		if ( BoundingBox::checkForIntersection(local_box,partition_box) )
		{
		    d_rcb_boxes.push_back( partition_box );
		    d_box_ranks.push_back( rank );
		}
	    }
	}
    }
    else
    {
	bounds[0] = -std::numeric_limits<double>::max();
	bounds[1] = -std::numeric_limits<double>::max();
	bounds[2] = -std::numeric_limits<double>::max();
	bounds[3] = std::numeric_limits<double>::max();
	bounds[4] = std::numeric_limits<double>::max();
	bounds[5] = std::numeric_limits<double>::max();
	d_rcb_boxes.push_back( BoundingBox(bounds) );
	d_box_ranks.push_back( 0 );
    }
}

//---------------------------------------------------------------------------//
/*!
 * \brief Given a range of local input point ids get their destination procs.
 *
 * \return The RCB destination procs for the points.
 */
Teuchos::Array<int> 
GeometryRCB::getInputPointDestinationProcs( const int lid_begin, 
					    const int num_points )
{
    Teuchos::Array<int> ranks( num_points, d_comm->getRank() );
    int lid = 0;
    for ( int i = 0; i < d_num_export; ++i )
    {
	lid = d_export_local_ids[i] - lid_begin;
	if ( 0 <= lid && num_points > lid )
	{
	    ranks[lid] = d_export_procs[i];
	}
    }

    return ranks;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Get the destination process for a point given its coordinates.
 *
 * \param coords Point coordinates. The dimension of the point must be equal
 * to or less than one.
 *
 * \return The GeometryRCB destination proc for the point.
 */
int GeometryRCB::getPointDestinationProc( 
    Teuchos::ArrayView<double> coords ) const
{
    DTK_REQUIRE( 0 <= coords.size() && coords.size() <= 3 );
    DTK_REQUIRE( d_dimension == Teuchos::as<int>(coords.size()) );

    for ( unsigned i = 0; i < d_rcb_boxes.size(); ++i )
    {
	if ( d_rcb_boxes[i].pointInBox(Teuchos::Array<double>(coords)) )
	{
	    return d_box_ranks[i];
	}
    }

    return -1;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Get the destination processes for a bounding box. This includes all
 * process domains that the box intersects.
 *
 * \param box The bounding box to get the destinations for.
 *
 * \return The GeometryRCB destination procs for the box
 */
Teuchos::Array<int>
GeometryRCB::getBoxDestinationProcs( const BoundingBox& box ) const
{
    Teuchos::Array<int> procs;

    for ( unsigned i = 0; i < d_rcb_boxes.size(); ++i )
    {
	if ( BoundingBox::checkForIntersection(box,d_rcb_boxes[i]) )
	{
	    procs.push_back( d_box_ranks[i] );
	}
    }

    return procs;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Zoltan callback for getting the number of geometry objects.
 */
int GeometryRCB::getNumberOfObjects( void *data, int *ierr )
{
    Teuchos::RCP<GeometryManager> geometry_manager = 
	*static_cast<Teuchos::RCP<GeometryManager>*>( data );
    int num_geometry = 0;

    // We'll only count geometry if the geometry manager is not null.
    if ( !geometry_manager.is_null() )
    {
	Teuchos::ArrayView<short int> active_geom = 
	    geometry_manager->getActiveGeometry();

	num_geometry = 
	    std::accumulate( active_geom.begin(), active_geom.end(), 0 );
    }

    *ierr = ZOLTAN_OK;

    // We're going to give zoltan the geometric bounding boxes so there will
    // be 8 points for each geometric object.
    return 8*num_geometry;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Zoltan callback for getting the local and global geometry ID's.
 */
void GeometryRCB::getObjectList( 
    void *data, int /*sizeGID*/, int /*sizeLID*/,
    ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
    int /*wgt_dim*/, float * /*obj_wgts*/, int *ierr )
{
    Teuchos::RCP<GeometryManager> geometry_manager = 
	*static_cast<Teuchos::RCP<GeometryManager>*>( data );

    // We'll only build the geometry list is the geometry manager is not null.
    if ( !geometry_manager.is_null() )
    {
	// Note here that the local ID is being set to the geometry array
	// index.
	zoltan_id_type i = 0;
	GlobalOrdinal lid = 0;
	Teuchos::ArrayView<short int> active_geom =
	    geometry_manager->getActiveGeometry();
	Teuchos::ArrayView<short int>::const_iterator active_it;
	Teuchos::ArrayRCP<GlobalOrdinal> local_gids = 
	    geometry_manager->gids();
	typename Teuchos::ArrayRCP<GlobalOrdinal>::const_iterator gid_it;
	typename Teuchos::ArrayRCP<GlobalOrdinal>::const_iterator gids_begin = 
	    local_gids.begin();
	for ( gid_it = local_gids.begin(), active_it = active_geom.begin();
	      gid_it != local_gids.end(); 
	      ++gid_it, ++active_it )
	{
	    lid = std::distance( gids_begin, gid_it );

	    if ( *active_it )
	    {
		globalID[i] = static_cast<zoltan_id_type>( *gid_it );
		localID[i] = static_cast<zoltan_id_type>( lid );
		++i;
	    }
	}
    }

    *ierr = ZOLTAN_OK;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Zoltan callback for getting the dimension of the vertices.
 */
int GeometryRCB::getNumGeometry( void *data, int *ierr )
{
    int dimension = *static_cast<int*>( data );
    *ierr = ZOLTAN_OK;
    return dimension;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Zoltan callback for getting the centroid coordinates.
 */
void GeometryRCB::getGeometryList(
    void *data, int sizeGID, int sizeLID,
    int num_obj,
    ZOLTAN_ID_PTR /*globalID*/, ZOLTAN_ID_PTR /*localID*/,
    int num_dim, double *geom_vec, int *ierr )
{
    Teuchos::RCP<GeometryManager> geometry_manager = 
	*static_cast<Teuchos::RCP<GeometryManager>*>( data );

    // We will only supply centroid coordinates when the geometry exists.
    if ( !geometry_manager.is_null() )
    {
	Teuchos::ArrayRCP<Geometry> local_geometry = 
	    geometry_manager->geometry();
	typename Teuchos::ArrayRCP<Geometry>::const_iterator geom_it;
	Teuchos::ArrayView<short int> active_geom =
	    geometry_manager->getActiveGeometry();
	Teuchos::ArrayView<short int>::const_iterator active_it;

	// Check Zoltan for consistency.
	int geom_dim = geometry_manager->dim();
	DTK_CHECK( sizeGID == 1 );
	DTK_CHECK( sizeLID == 1 );
	DTK_CHECK( num_dim == Teuchos::as<int>(geom_dim) );

	if ( sizeGID != 1 || sizeLID != 1 || 
	     num_dim != Teuchos::as<int>(geom_dim) )
	{
	    *ierr = ZOLTAN_FATAL;
	    return;
	}
    
	// Zoltan needs interleaved coordinates.
	int n = 0;
	Teuchos::Tuple<double,6> geometry_bounds;
	for ( geom_it = local_geometry.begin(), active_it = active_geom.begin();
	      geom_it != local_geometry.end();
	      ++geom_it, ++active_it )
	{
	    if ( *active_it )
	    {
		// Get the bounding box.
		geometry_bounds = GT::boundingBox( *geom_it ).getBounds();

		// lo x, lo y, lo z
		geom_vec[ geom_dim*n ] = geometry_bounds[0];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[1];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[2];
		++n;

		// hi x, lo y, lo z
		geom_vec[ geom_dim*n ] = geometry_bounds[3];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[1];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[2];
		++n;

		// lo x, hi y, lo z
		geom_vec[ geom_dim*n ] = geometry_bounds[0];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[4];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[2];
		++n;

		// hi x, hi y, lo z
		geom_vec[ geom_dim*n ] = geometry_bounds[3];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[4];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[2];
		++n;

		// lo x, lo y, hi z
		geom_vec[ geom_dim*n ] = geometry_bounds[0];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[1];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[5];
		++n;

		// hi x, lo y, hi z
		geom_vec[ geom_dim*n ] = geometry_bounds[3];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[1];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[5];
		++n;

		// lo x, hi y, hi z
		geom_vec[ geom_dim*n ] = geometry_bounds[0];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[4];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[5];
		++n;

		// hi x, hi y, hi z
		geom_vec[ geom_dim*n ] = geometry_bounds[3];
		if ( geom_dim < 1 )
		    geom_vec[ geom_dim*n + 1] = geometry_bounds[4];
		if ( geom_dim < 2 )
		    geom_vec[ geom_dim*n + 2] = geometry_bounds[5];
		++n;
	    }
	}

	DTK_ENSURE( num_obj == n );
    }
	  
    *ierr = ZOLTAN_OK;
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//

#endif // end DTK_GEOMETRYRCB_DEF_HPP

//---------------------------------------------------------------------------//
// end DTK_GeometryRCB_def.hpp
//---------------------------------------------------------------------------//