/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkTransformIOTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkAffineTransform.h"
#include "itkSimilarity2DTransform.h"
#include "itkBSplineDeformableTransform.h"

int itkTransformIOTest(int itkNotUsed(ac), char* itkNotUsed(av)[])
{
  typedef itk::AffineTransform<double,3> AffineTransformType;
  AffineTransformType::Pointer affine = AffineTransformType::New();
  AffineTransformType::InputPointType cor;
  cor[0] = 1.0;
  cor[1] = 2.0;
  cor[2] = 3.0;
  affine->SetCenter(cor);

  itk::TransformFileWriter::Pointer writer;
  itk::TransformFileReader::Pointer reader;
  reader = itk::TransformFileReader::New();
  writer = itk::TransformFileWriter::New();
  writer->AddTransform(affine);

  writer->SetFileName( "Transforms.txt" );
  reader->SetFileName( "Transforms.txt" );
 
  // Testing writing
  std::cout << "Testing write : ";
  affine->Print ( std::cout );
  try
   {
   writer->Update();
   std::cout << std::endl;
   std::cout << "Testing read : " << std::endl;
   reader->Update();
   }
  catch( itk::ExceptionObject & excp )
   {
   std::cerr << "Error while saving the transforms" << std::endl;
   std::cerr << excp << std::endl;
   std::cout << "[FAILED]" << std::endl;
   return EXIT_FAILURE;
   }


  try
    {
    itk::TransformFileReader::TransformListType *list;
    list = reader->GetTransformList();
    itk::TransformFileReader::TransformListType::iterator i = list->begin();
    while ( i != list->end() )
      {
      (*i)->Print ( std::cout );
      i++;
      }
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  
  std::cout << "[PASSED]" << std::endl;

  return EXIT_SUCCESS;
}
