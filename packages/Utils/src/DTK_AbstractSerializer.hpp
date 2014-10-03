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
 * \brief DTK_AbstractSerializer.hpp
 * \author Stuart R. Slattery
 * \brief Serializable object policy interface.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_ABSTRACTSERIALIZER_HPP
#define DTK_ABSTRACTSERIALIZER_HPP

#include <string>

#include "DTK_AbstractSerializableObjectPolicy.hpp"

#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayView.hpp>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
  \class AbstractSerializer
  \brief Serializer for abstract objects.

  The data packets are reference counted pointers to the base class
  objects. Abstract classes that can implement the
  AbstractSerializableObjectPolicy can use this class to quickly implement
  Teuchos::SerializationTraits. This object mimics the Teuchos indirect
  serialization traits.
*/
//---------------------------------------------------------------------------//
template<class Ordinal, class T>
class AbstractSerializer
{
  public:

    //@{
    //! Typdefs.
    typedef Ordinal                             ordinal_type;
    typedef Teuchos::RCP<T>                     Packet;
    typedef AbstractSerializableObjectPolicy<T> ASOP;
    //@}

  public:

    // Return the number of bytes for count objects.
    static Ordinal fromCountToIndirectBytes( const Ordinal count, 
					     const Packet buffer[] );

    // Serialize to an indirect char buffer.
    static void serialize( const Ordinal count, 
			   const Packet buffer[], 
			   const Ordinal bytes, 
			   char charBuffer[] );

    // Return the number of objects for bytes of storage.
    static Ordinal fromIndirectBytesToCount( const Ordinal bytes, 
					     const char charBuffer[] );

    // Deserialize from an indirect char buffer.
    static void deserialize( const Ordinal bytes, 
			     const char charBuffer[], 
			     const Ordinal count, 
			     Packet buffer[] );

  public:

    // Direct serialization support.
    static const bool supportsDirectSerialization;
};

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// Template includes.
//---------------------------------------------------------------------------//

#include "DTK_AbstractSerializer_impl.hpp"

//---------------------------------------------------------------------------//

#endif // end DTK_ABSTRACTSERIALIZER_HPP

//---------------------------------------------------------------------------//
// end DTK_AbstractSerializer.hpp
//---------------------------------------------------------------------------//