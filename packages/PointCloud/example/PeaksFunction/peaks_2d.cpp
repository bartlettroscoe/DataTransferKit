//---------------------------------------------------------------------------//
/*!
 * \file peaks_2d.cpp
 * \author Stuart R. Slattery
 * \brief Spline interpolation example.
 */
//---------------------------------------------------------------------------//

#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <DTK_PointCloudInterpolator.hpp>
#include <DTK_PointCloudInterpolatorFactory.hpp>

#include <Teuchos_Array.hpp>
#include <Teuchos_TimeMonitor.hpp>
#include <Teuchos_Time.hpp>
#include <Teuchos_ParameterList.hpp>

#include "Teuchos_RCP.hpp"
#include "Teuchos_Ptr.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_ArrayView.hpp"
#include "Teuchos_Comm.hpp"
#include "Teuchos_DefaultComm.hpp"

//---------------------------------------------------------------------------//
// HELPER FUNCTIONS
//---------------------------------------------------------------------------//
// Peaks function.
double peaks( const double x, const double y )
{
    return 3.0*(1.0-x)*(1.0-x)*std::exp(-x*x-(y+1.0)*(y+1.0)) -
	10.0*(x/5.0-x*x*x-y*y*y*y*y)*std::exp(-x*x-y*y) -
	(1.0/3.0)*std::exp(-(x+1.0)*(x+1.0)-y*y);
}

//---------------------------------------------------------------------------//
int main( int argc, char * argv[] )
{
    // Problem spatial dimension.
    const int space_dim = 2;

    // Initialize communication.
    Teuchos::GlobalMPISession mpi_session( &argc, &argv );
    Teuchos::RCP<const Teuchos::Comm<int> > comm =
	Teuchos::DefaultComm<int>::getComm();

    // Setup the source centers.
    int N = 100;
    Teuchos::Array<double> source_centers( space_dim*N*N );
    Teuchos::Array<unsigned> source_center_ids( N*N );
    double h = 6.0 / N;
    for ( int i = 0; i < N; ++i )
    {
	for ( int j = 0; j < N; ++j )
	{
	    source_centers[ (i*N + j)*space_dim ] = -3.0 + h*i;
	    source_centers[ (i*N + j)*space_dim + 1 ] = -3.0 + h*j;
	    source_center_ids[ i*N + j ] = i*N + j;
	}
    }

    // Set the source function values on the source centers.
    Teuchos::Array<double> source_function( 2*N*N );
    for ( int i = 0; i < N; ++i )
    {
	for ( int j = 0; j < N; ++j )
	{
	    source_function[ i*N + j ] = 
		peaks( source_centers[ (i*N + j)*space_dim ],
		       source_centers[ (i*N + j)*space_dim + 1] );

	    source_function[ i*N + j + N*N ] = 
		source_function[ i*N + j ];
	}
    }

    // Build the target centers in between the source centers.
    Teuchos::Array<double> target_centers( space_dim*(N-1)*(N-1) );
    for ( int i = 0; i < N-1; ++i )
    {
	for ( int j = 0; j < N-1; ++j )
	{
	    target_centers[ (i*(N-1) + j)*space_dim ] = -3.0 + h*i + 0.5*h;
	    target_centers[ (i*(N-1) + j)*space_dim + 1 ] = -3.0 + h*j + 0.5*h;
	}
    }

    // Allocate memory for the target function.
    Teuchos::Array<double> target_function( 2*(N-1)*(N-1) );

    // Support radius.
    double radius = 2.0*h;

    // Interpolation type.
    std::string interpolation_type = "Moving Least Square";

    // Basis Type.
    std::string basis_type = "Wendland";

    // Interpolation basis order.
    const int basis_order = 2;

    // Data space_dimension.
    const int data_dim = 2;

    // Build the interpolation object.
    typedef int GlobalOrdinal;
    Teuchos::RCP<DataTransferKit::PointCloudInterpolator> interpolator =
	DataTransferKit::PointCloudInterpolatorFactory::create<GlobalOrdinal>(
	    comm, interpolation_type, basis_type, basis_order, space_dim );

    // Assign the source centers, target centers, and support radius to the
    // interpolator.
    std::cout << "Building DataTransferKit::SplineInterpolator" << std::endl;
    interpolator->setProblem( source_centers(), target_centers(), radius );

    // Perform the interpolation.
    std::cout << "Performing interpolation" << std::endl;
    interpolator->interpolate( source_function(), target_function(), data_dim );

    // Calculate the true solution at the target centers.
    Teuchos::Array<double> true_solution( (N-1)*(N-1) );
    for ( int i = 0; i < N-1; ++i )
    {
	for ( int j = 0; j < N-1; ++j )
	{
	    true_solution[ i*(N-1) + j ] = 
		peaks( target_centers[ (i*(N-1) + j)*space_dim ],
		       target_centers[ (i*(N-1) + j)*space_dim + 1] );
	}
    }
    
    // Calculate the point-wise error.
    Teuchos::Array<double> error_1( (N-1)*(N-1) );
    Teuchos::Array<double> error_2( (N-1)*(N-1) );
    for ( unsigned i = 0; i < true_solution.size(); ++i )
    {
	error_1[i] = target_function[i] - true_solution[i];
	error_2[i] = target_function[i+true_solution.size()] - true_solution[i];
    }

    // Calculate the 2-norm of the error.
    double error_norm_1 = 0.0;
    double error_norm_2 = 0.0;
    for ( unsigned i = 0; i < error_1.size(); ++i )
    {
	error_norm_1 += error_1[i]*error_1[i];
	error_norm_2 += error_2[i]*error_2[i];
    }
    error_norm_1 = std::sqrt(error_norm_1);
    error_norm_2 = std::sqrt(error_norm_2);
    std::cout << "||e1||_2 = " << error_norm_1 << std::endl;
    std::cout << "||e2||_2 = " << error_norm_2 << std::endl;

    // Write the solutions to a VTK file.
    std::ofstream vtk_file;
    vtk_file.open( "peaks_2d.vtk" );

    // File header.
    vtk_file << "# vtk DataFile Version 2.0\n";
    vtk_file << "DataTransferKit version 0.0\n";
    vtk_file << "ASCII\n";
    vtk_file << "DATASET RECTILINEAR_GRID\n";
    vtk_file << "DIMENSIONS " << N << " " << N << " 1\n";

    // Set the grid
    vtk_file << "X_COORDINATES " << N << " double\n";
    for ( int i = 0; i < N; ++i )
    {
	vtk_file << -3.0 + h*i << " ";
    }
    vtk_file << "\n";

    vtk_file << "Y_COORDINATES " << N << " double\n";
    for ( int j = 0; j < N; ++j )
    {
	vtk_file << -3.0 + h*j << " ";
    }
    vtk_file << "\n";

    vtk_file << "Z_COORDINATES 1 double\n";
    vtk_file << "0\n";

    // Set the source function.
    vtk_file << "POINT_DATA " << N*N << "\n";
    vtk_file << "SCALARS " << "Source" << " double " << 1 << "\n";
    vtk_file << "LOOKUP_TABLE default\n";
    for ( int i = 0; i < N; ++i )
    {
	for ( int j = 0; j < N; ++j )
	{
	    vtk_file << source_function[ i*N + j ] << "\n";
	}
    }

    // Set the target function.
    vtk_file << "CELL_DATA " << (N-1)*(N-1) << "\n";
    vtk_file << "SCALARS " << "Target" << " double " << 1 << "\n";
    vtk_file << "LOOKUP_TABLE default\n";
    for ( int i = 0; i < N-1; ++i )
    {
	for ( int j = 0; j < N-1; ++j )
	{
	    vtk_file << target_function[ i*(N-1) + j ] << "\n";
	}
    }

    // Set the error.
    vtk_file << "SCALARS " << "Error" << " double " << 1 << "\n";
    vtk_file << "LOOKUP_TABLE default\n";
    for ( int i = 0; i < N-1; ++i )
    {
	for ( int j = 0; j < N-1; ++j )
	{
	    vtk_file << error_1[ i*(N-1) + j ] << "\n";
	}
    }

    // Close the file.
    vtk_file.close();

    return 0;
}

//---------------------------------------------------------------------------//
// end peaks_2d.cpp
//---------------------------------------------------------------------------//

