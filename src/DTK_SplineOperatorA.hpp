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
 * \file   DTK_SplineOperatorA.hpp
 * \author Stuart R. Slattery
 * \brief  Spline transformation operator.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_SPLINEOPERATORA_HPP
#define DTK_SPLINEOPERATORA_HPP

#include "DTK_RadialBasisPolicy.hpp"
#include "DTK_SplineInterpolationPairing.hpp"

#include <Teuchos_ArrayView.hpp>
#include <Teuchos_RCP.hpp>

#include <Tpetra_Map.hpp>
#include <Tpetra_MultiVector.hpp>
#include <Tpetra_CrsMatrix.hpp>
#include <Tpetra_Operator.hpp>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
 * \class SplineOperatorA
 * \brief Sparse spline transformation operator (the A matrix). A = N + Q
 */
//---------------------------------------------------------------------------//
template<class Basis, class GO, int DIM>
class SplineOperatorA : public Tpetra::Operator<double,int,GO>
{
  public:

    //@{
    //! Typedefs.
    typedef RadialBasisPolicy<Basis> BP;
    //@}

    // Constructor.
    SplineOperatorA(
	Teuchos::RCP<const Tpetra::Map<int,GO> >& domain_map,
	Teuchos::RCP<const Tpetra::Map<int,GO> >& range_map,
	const Teuchos::ArrayView<const double>& target_centers,
	const Teuchos::ArrayView<const GO>& target_center_gids,
	const Teuchos::ArrayView<const double>& dist_source_centers,
	const Teuchos::ArrayView<const GO>& dist_source_center_gids,
	const Teuchos::RCP<SplineInterpolationPairing<DIM> >& target_pairings,
	const Basis& basis );

    //! Destructor.
    ~SplineOperatorA()
    { /* ... */ }

    //! The Map associated with the domain of this operator, which must be
    //! compatible with X.getMap().
    Teuchos::RCP<const Tpetra::Map<int,GO> > getDomainMap() const
    { return d_N->getDomainMap(); }

    //! The Map associated with the range of this operator, which must be
    //! compatible with Y.getMap().
    Teuchos::RCP<const Tpetra::Map<int,GO> > getRangeMap() const
    { return d_N->getRangeMap(); }

    //! \brief Computes the operator-multivector application.
    /*! Loosely, performs \f$Y = \alpha \cdot A^{\textrm{mode}} \cdot X +
        \beta \cdot Y\f$. However, the details of operation vary according to
        the values of \c alpha and \c beta. Specifically - if <tt>beta ==
        0</tt>, apply() <b>must</b> overwrite \c Y, so that any values in \c Y
        (including NaNs) are ignored.  - if <tt>alpha == 0</tt>, apply()
        <b>may</b> short-circuit the operator, so that any values in \c X
        (including NaNs) are ignored.
     */
    void apply (const Tpetra::MultiVector<double,int,GO> &X,
		Tpetra::MultiVector<double,int,GO> &Y,
		Teuchos::ETransp mode = Teuchos::NO_TRANS,
		double alpha = Teuchos::ScalarTraits<double>::one(),
		double beta = Teuchos::ScalarTraits<double>::zero()) const;

    /// \brief Whether this operator supports applying the transpose or
    /// conjugate transpose.
    bool hasTransposeApply() const
    { return false; }

  private:

    // The N matrix.
    Teuchos::RCP<Tpetra::CrsMatrix<double,int,GO> > d_N;

    // The Q matrix.
    Teuchos::RCP<Tpetra::CrsMatrix<double,int,GO> > d_Q;
};

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// Template includes.
//---------------------------------------------------------------------------//

#include "DTK_SplineOperatorA_impl.hpp"

//---------------------------------------------------------------------------//

#endif // end DTK_SPLINEOPERATORA_HPP

//---------------------------------------------------------------------------//
// end DTK_SplineOperatorA.hpp
//---------------------------------------------------------------------------//

