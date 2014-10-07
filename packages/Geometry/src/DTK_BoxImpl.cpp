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
  * \file DTK_BoxImpl.cpp
  * \author Stuart R. Slattery
  * \brief Box implementation.
  */
//---------------------------------------------------------------------------//

#include "DTK_BoxImpl.hpp"
#include "DTK_DBC.hpp"
#include "DTK_DataSerializer.hpp"

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
 * \brief Default constructor.
 */
BoxImpl::BoxImpl()
    : d_global_id( dtk_invalid_entity_id )
    , d_owner_rank( -1 )
    , d_x_min( 0.0 )
    , d_y_min( 0.0 )
    , d_z_min( 0.0 )
    , d_x_max( 0.0 )
    , d_y_max( 0.0 )
    , d_z_max( 0.0 )
{ 
    d_centroid[0] = 0.0;
    d_centroid[1] = 0.0;
    d_centroid[2] = 0.0;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Constructor.
 *
 * \param x_min Minimum x coordinate value in the box.
 *
 * \param y_min Minimum y coordinate value in the box.
 *
 * \param z_min Minimum z coordinate value in the box.
 *
 * \param x_max Maximum x coordinate value in the box.
 *
 * \param y_max Maximum y coordinate value in the box.
 *
 * \param z_max Maximum z coordinate value in the box.
 */
BoxImpl::BoxImpl( const EntityId global_id, const int owner_rank,
		  const double x_min, const double y_min, const double z_min,
		  const double x_max, const double y_max, const double z_max )
    : d_global_id( global_id )
    , d_owner_rank( owner_rank )
    , d_x_min( x_min )
    , d_y_min( y_min )
    , d_z_min( z_min )
    , d_x_max( x_max )
    , d_y_max( y_max )
    , d_z_max( z_max )
{
    d_centroid[0] = (d_x_max + d_x_min) / 2.0;
    d_centroid[1] = (d_y_max + d_y_min) / 2.0;
    d_centroid[2] = (d_z_max + d_z_min) / 2.0;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Tuple constructor.
 *
 * \param bounds Tuple containing {x_min, y_min, z_min, x_max, y_max, z_max}.
 */
BoxImpl::BoxImpl( const EntityId global_id,
		  const int owner_rank, 
		  const Teuchos::Tuple<double,6>& bounds )
    : d_global_id( global_id )
    , d_owner_rank( owner_rank )
    , d_x_min( bounds[0] )
    , d_y_min( bounds[1] )
    , d_z_min( bounds[2] )
    , d_x_max( bounds[3] )
    , d_y_max( bounds[4] )
    , d_z_max( bounds[5] )
{
    d_centroid[0] = (d_x_max + d_x_min) / 2.0;
    d_centroid[1] = (d_y_max + d_y_min) / 2.0;
    d_centroid[2] = (d_z_max + d_z_min) / 2.0;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Destructor.
 */
BoxImpl::~BoxImpl()
{ /* ... */ }

//---------------------------------------------------------------------------//
// Return a string indicating the derived entity type.
std::string BoxImpl::entityType() const
{
    return std::string("DTK Box");
}

//---------------------------------------------------------------------------//
// Get the unique global identifier for the entity.
EntityId BoxImpl::id() const
{
    return d_global_id;
}
    
//---------------------------------------------------------------------------//
// Get the parallel rank that owns the entity.
int BoxImpl::ownerRank() const
{
    return d_owner_rank;
}

//---------------------------------------------------------------------------//
// Return the physical dimension of the entity.
int BoxImpl::physicalDimension() const
{
    return 3;
}

//---------------------------------------------------------------------------//
// Return the parametric dimension of the entity.
int BoxImpl::parametricDimension() const
{
    return 3;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Compute the volume of the box.
 *
 * \return Return the volume of the box.
 */
double BoxImpl::measure() const
{
    return (d_x_max-d_x_min)*(d_y_max-d_y_min)*(d_z_max-d_z_min);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Get the centroid of the box.
 *
 * \return The centroid coordinates.
 */
void BoxImpl::centroid( Teuchos::ArrayView<const double>& centroid ) const
{
    centroid = Teuchos::ArrayView<const double>(d_centroid,3);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Compute the bounding box around the box.
 *
 * \return The bounding box bounds.
 */
void BoxImpl::boundingBox( Teuchos::Tuple<double,6>& bounds ) const
{
    bounds = getBounds();
}

//---------------------------------------------------------------------------//
/*!
 * \brief Map a point to the reference space of an entity. Return the
 * parameterized point.
 */
void BoxImpl::safeguardMapToReferenceFrame(
    const Teuchos::ParameterList& parameters,
    const Teuchos::ArrayView<const double>& point,
    MappingStatus& status ) const
{
    status.mappingSucceeded();
}

//---------------------------------------------------------------------------//
/*!
 * \brief Map a point to the reference space of an entity. Return the
 */
void BoxImpl::mapToReferenceFrame( 
    const Teuchos::ParameterList& parameters,
    const Teuchos::ArrayView<const double>& point,
    const Teuchos::ArrayView<double>& reference_point,
    MappingStatus& status ) const
{
    reference_point.assign( point );
    status.mappingSucceeded();
}

//---------------------------------------------------------------------------//
/*!  
 * \brief Determine if a reference point is in the parameterized space of
 * an entity.
 */
bool BoxImpl::checkPointInclusion( 
    const Teuchos::ParameterList& parameters,
    const Teuchos::ArrayView<const double>& reference_point ) const
{
    DTK_REQUIRE( 3 == reference_point.size() );

    double tolerance = parameters.get<double>("Inclusion Tolerance");

    double x_tol = (d_x_max - d_x_min)*tolerance;
    double y_tol = (d_y_max - d_y_min)*tolerance;
    double z_tol = (d_z_max - d_z_min)*tolerance;

    if ( reference_point[0] >= d_x_min - x_tol &&
	 reference_point[1] >= d_y_min - y_tol &&
	 reference_point[2] >= d_z_min - z_tol &&
	 reference_point[0] <= d_x_max + x_tol &&
	 reference_point[1] <= d_y_max + y_tol &&
	 reference_point[2] <= d_z_max + z_tol )
    {
	return true;
    }

    return false;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Map a reference point to the physical space of an entity.
 */
void BoxImpl::mapToPhysicalFrame( 
    const Teuchos::ArrayView<const double>& reference_point,
    const Teuchos::ArrayView<double>& point ) const
{
    point.assign( reference_point );
}

//---------------------------------------------------------------------------//
// Serialize the entity into a buffer.
void BoxImpl::serialize( const Teuchos::ArrayView<char>& buffer ) const
{
    DTK_REQUIRE( Teuchos::as<std::size_t>(buffer.size()) >= BoxImpl::byteSize() );
    DataSerializer serializer;
    serializer.setBuffer( buffer );
    serializer << d_global_id << d_owner_rank
	       << d_x_min << d_y_min << d_z_min
	       << d_x_max << d_y_max << d_z_max;
}

//---------------------------------------------------------------------------//
// Deserialize an entity from a buffer.
void BoxImpl::deserialize( const Teuchos::ArrayView<const char>& buffer )
{
    // Unpack the data.
    DTK_REQUIRE( Teuchos::as<std::size_t>(buffer.size()) >= BoxImpl::byteSize() );
    DataDeserializer deserializer;
    Teuchos::ArrayView<char> buffer_nonconst(
	const_cast<char*>(buffer.getRawPtr()), buffer.size() );
    deserializer.setBuffer( buffer_nonconst );
    deserializer >> d_global_id >> d_owner_rank
		 >> d_x_min >> d_y_min >> d_z_min
		 >> d_x_max >> d_y_max >> d_z_max;

    // Build the centroid.
    d_centroid[0] = (d_x_max + d_x_min) / 2.0;
    d_centroid[1] = (d_y_max + d_y_min) / 2.0;
    d_centroid[2] = (d_z_max + d_z_min) / 2.0;
}

//---------------------------------------------------------------------------//
// Static members.
//---------------------------------------------------------------------------//
// Byte size of the object.
std::size_t 
BoxImpl::d_byte_size = sizeof(EntityId) + sizeof(int) + 6*sizeof(double);

//---------------------------------------------------------------------------//
// Get the byte size of the box.
std::size_t BoxImpl::byteSize()
{
    return d_byte_size;
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// end DTK_BoxImpl.cpp
//---------------------------------------------------------------------------//
