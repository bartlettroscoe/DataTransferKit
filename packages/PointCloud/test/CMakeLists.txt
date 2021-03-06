INCLUDE(TribitsAddExecutableAndTest)
INCLUDE(TribitsAddAdvancedTest)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  RadialBasis_test
  SOURCES tstRadialBasis.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  CloudDomain_test
  SOURCES tstCloudDomain.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  SplineInterpolationPairing_test
  SOURCES tstSplineInterpolationPairing.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  CenterDistributor_test
  SOURCES tstCenterDistributor.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  PolynomialMatrix_test
  SOURCES tstPolynomialMatrix.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  SplineProlongationOperator_test
  SOURCES tstSplineProlongationOperator.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  SplineCoefficientMatrix_test
  SOURCES tstSplineCoefficientMatrix.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  SplineInterpolation_test
  SOURCES tstSplineInterpolation.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  MovingLeastSquare_test
  SOURCES tstMovingLeastSquare.cpp ${TEUCHOS_STD_PARALLEL_UNIT_TEST_MAIN}
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_COPY_FILES_TO_BINARY_DIR(
  SplineInterpolationXML
  SOURCE_FILES spline_interpolation_test.xml
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}
  EXEDEPS SplineInterpolation_test
)
