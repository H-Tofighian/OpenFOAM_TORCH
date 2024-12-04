/*---------------------------------------------------------------------------*\
    FoamToNumpy - OpenFOAM to NumPy Data Converter

    MIT License

    Copyright (c) 2024 H. Tofighian

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    This utility uses OpenFOAM libraries:
    Copyright (C) 2016-2021 OpenFOAM Foundation & OpenCFD Ltd.

    Application
        FoamToNumpy
    Description
        Converts OpenFOAM vector fields to binary format readable by NumPy.
        Currently supports the velocity field (U) and cell centers (cellC).
        
        The output format is a binary file containing float32 data in C-style
        row-major order. For each time step, two files are created:
        - U_flat_{time}.bin: Contains flattened velocity vectors [u1,v1,w1,u2,v2,w2,...]
        - cellC_flat_{time}.bin: Contains flattened cell centers [x1,y1,z1,x2,y2,z2,...]
    
        Usage:
            FoamToNumpy -time <time>
    
    Author
        H. Tofighian
\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "timeSelector.H"
#include "volFields.H"

#include <stdio.h>
#include <stdlib.h>

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();

    #include "setRootCase.H"
    #include "createTime.H"
    instantList timeDirs = timeSelector::select0(runTime, args);
    #include "createNamedMesh.H"
    

    int n = mesh.cells().size(); 
    float* U_flat = new float[3*n];
    float* cellC_flat = new float[3*n];
    
    forAll(timeDirs, timei)
    {
        runTime.setTime(timeDirs[timei], timei);

        Info<< "Time = " << runTime.timeName() << endl;
        Info<< "Reading field U\n" << endl;
        volVectorField U
        (
            IOobject
            (
                "U",
                runTime.timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            mesh
        );
        
        volVectorField cellC
        (
            IOobject
            (
                "cellC",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh.C()
        );
        
        int idx = 0;
        
        forAll(U, i)
        {
			cellC_flat[idx] = cellC[i].x();
            U_flat[idx++] = U[i].x();
            cellC_flat[idx] = cellC[i].y();
            U_flat[idx++] = U[i].y();
            cellC_flat[idx] = cellC[i].z();
            U_flat[idx++] = U[i].z();
		}
		
		std::string filename = "U_flat_" + runTime.timeName() + ".bin";
		FILE *fp = fopen(filename.c_str(), "wb");
		if (fp == NULL)
		{
			printf("Failed to open file for writing\n");
			exit(1);
		}
		fwrite(U_flat, sizeof(float), 3*n, fp);
		fclose(fp);
		
		
		std::string filename2 = "cellC_flat_" + runTime.timeName() + ".bin";
		FILE *fp2 = fopen(filename2.c_str(), "wb");
		if (fp2 == NULL)
		{
			printf("Failed to open file for writing\n");
			exit(1);
		}
		fwrite(cellC_flat, sizeof(float), 3*n, fp2);
		fclose(fp2);
		
		
		cellC.write();
    }
    
    delete[] U_flat;
    delete[] cellC_flat;
    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
