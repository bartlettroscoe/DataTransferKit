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
 * \file DTK_Box.hpp
 * \author Stuart R. Slattery
 * \brief Bounding box declaration.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_BOX_HPP
#define DTK_BOX_HPP

#include "DTK_GeometricEntity.hpp"

#include <Teuchos_Tuple.hpp>
#include <Teuchos_Array.hpp>
#include <Teuchos_SerializationTraits.hpp>
#include <Teuchos_ParameterList.hpp>

#include <iostream>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
 * \class Box
 * \brief Axis-aligned Cartesian box container.
 *
 * All three dimensions are explictly represented in this bounding box. This
 * is different from a bounding box in that it must always be finite and of a
 * fixed 3 dimensions.
 */
//---------------------------------------------------------------------------//
class Box : public GeometricEntity
{

  public:

    // Default constructor.
    Box();

    // Constructor.
    Box( const EntityId global_id, const int owner_rank,
	 const double x_min, const double y_min, const double z_min,
	 const double x_max, const double y_max, const double z_max );

    // Tuple constructor.
    Box( const EntityId global_id,
	 const int owner_rank, 
	 const Teuchos::Tuple<double,6>& bounds );

    // Destructor.
    ~Box();

    // Get the boundaries of the box.
    Teuchos::Tuple<double,6> getBounds() const
    { return Teuchos::tuple( d_x_min, d_y_min, d_z_min, 
			     d_x_max, d_y_max, d_z_max ); }

    //@{ 
    //! GeometricEntity implementation.
    // Return a string indicating the derived entity type.
    std::string entityType() const;

    // Get the unique global identifier for the entity.
    EntityId id() const;
    
    // Get the parallel rank that owns the entity.
    int ownerRank() const;

    // Return the physical dimension of the entity.
    int physicalDimension() const;

    // Return the parametric dimension of the entity.
    int parametricDimension() const;

    // Return the entity measure with respect to the parameteric
    double measure() const;

    // Return the centroid of the entity.
    void centroid( Teuchos::ArrayView<const double>& centroid ) const;

    // Return the axis-aligned bounding box around the entity.
    void boundingBox( Box& bounding_box ) const;

    // Perform a safeguard check for mapping a point to the reference
    void safeguardMapToReferenceFrame(
	const Teuchos::ParameterList& parameters,
	const Teuchos::ArrayView<const double>& point,
	MappingStatus& status ) const;

    // Map a point to the reference space of an entity. Return the
    void mapToReferenceFrame( 
	const Teuchos::ParameterList& parameters,
	const Teuchos::ArrayView<const double>& point,
	const Teuchos::ArrayView<double>& reference_point,
	MappingStatus& status ) const;

    // Determine if a reference point is in the parameterized space of
    bool checkPointInclusion( 
	const Teuchos::ParameterList& parameters,
	const Teuchos::ArrayView<const double>& reference_point ) const;

    // Map a reference point to the physical space of an entity.
    void mapToPhysicalFrame( 
	const Teuchos::ArrayView<const double>& reference_point,
	const Teuchos::ArrayView<double>& point ) const;
     
    // Get the size of the serialized entity in bytes.
    std::size_t byteSize() const;

    // Serialize the entity into a buffer.
    void serialize( const Teuchos::ArrayView<char>& buffer ) const;

    // Deserialize an entity from a buffer.
    void deserialize( const Teuchos::ArrayView<const char>& buffer );
    //@}

    // Static function to check for box intersection but not perform it.
    static bool checkForIntersection( const Box& box_A,
				      const Box& box_B );

    // Static function for box intersection.
    static bool intersectBoxes( const Box& box_A,
				const Box& box_B,
				Box& box_intersection );

    // Static function for box union
    static void uniteBoxes( const Box& box_A,
			    const Box& box_B,
			    Box& box_union );

  private:

    // Global id.
    EntityId d_global_id;

    // Owning parallel rank.
    int d_owner_rank;

    // X min.
    double d_x_min;

    // Y min.
    double d_y_min;

    // Z min.
    double d_z_min;

    // X max.
    double d_x_max;

    // Y max.
    double d_y_max;

    // Z max.
    double d_z_max;

    // Packed size in bytes.
    std::size_t d_byte_size;

    // Centroid coordinates.
    double d_centroid[3];
};

//---------------------------------------------------------------------------//
//! overload for printing box
std::ostream& operator<< (std::ostream& os,const DataTransferKit::Box& b); 

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// Direct Serialization Traits Specialization.
//---------------------------------------------------------------------------//

namespace Teuchos
{

template<typename Ordinal>
class SerializationTraits<Ordinal, DataTransferKit::Box>
    : public DirectSerializationTraits<Ordinal, DataTransferKit::Box>
{};

} // end namespace Teuchos

//---------------------------------------------------------------------------//

#endif // end DTK_BOX_HPP

//---------------------------------------------------------------------------//
// end DTK_Box.hpp
//---------------------------------------------------------------------------//
