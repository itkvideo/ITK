/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkResampleImageFilter_txx
#define __itkResampleImageFilter_txx

#include "itkResampleImageFilter.h"
#include "itkObjectFactory.h"
#include "itkIdentityTransform.h"
#include "itkProgressReporter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkSpecialCoordinatesImage.h"

namespace itk
{
/**
 * Initialize new instance
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::ResampleImageFilter()
{
  m_OutputOrigin.Fill(0.0);
  m_OutputSpacing.Fill(1.0);
  m_OutputDirection.SetIdentity();

  m_UseReferenceImage = false;

  m_Size.Fill(0);
  m_OutputStartIndex.Fill(0);

  m_Transform =
    IdentityTransform< TInterpolatorPrecisionType, ImageDimension >::New();

  m_InterpolatorIsBSpline = false;
  m_BSplineInterpolator = NULL;

  m_Interpolator = dynamic_cast< InterpolatorType * >
                   ( LinearInterpolatorType::New().GetPointer() );

  m_DefaultPixelValue = 0;
}

/**
 * Print out a description of self
 *
 * \todo Add details about this class
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "DefaultPixelValue: "
     << static_cast< typename NumericTraits< PixelType >::PrintType >
  ( m_DefaultPixelValue )
     << std::endl;
  os << indent << "Size: " << m_Size << std::endl;
  os << indent << "OutputStartIndex: " << m_OutputStartIndex << std::endl;
  os << indent << "OutputSpacing: " << m_OutputSpacing << std::endl;
  os << indent << "OutputOrigin: " << m_OutputOrigin << std::endl;
  os << indent << "OutputDirection: " << m_OutputDirection << std::endl;
  os << indent << "Transform: " << m_Transform.GetPointer() << std::endl;
  os << indent << "Interpolator: " << m_Interpolator.GetPointer() << std::endl;
  os << indent << "UseReferenceImage: " << ( m_UseReferenceImage ? "On" : "Off" )
     << std::endl;
  return;
}

/**
 * Set the output image spacing.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::SetOutputSpacing(const double *spacing)
{
  SpacingType s(spacing);

  this->SetOutputSpacing(s);
}

/**
 * Set the output image origin.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::SetOutputOrigin(const double *origin)
{
  OriginPointType p(origin);

  this->SetOutputOrigin(p);
}

/** Helper method to set the output parameters based on this image */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::SetOutputParametersFromImage(const ImageBaseType *image)
{
  this->SetOutputOrigin ( image->GetOrigin() );
  this->SetOutputSpacing ( image->GetSpacing() );
  this->SetOutputDirection ( image->GetDirection() );
  this->SetOutputStartIndex ( image->GetLargestPossibleRegion().GetIndex() );
  this->SetSize ( image->GetLargestPossibleRegion().GetSize() );
}

/**
 * Set up state of filter before multi-threading.
 * InterpolatorType::SetInputImage is not thread-safe and hence
 * has to be set up before ThreadedGenerateData
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::BeforeThreadedGenerateData()
{
  if ( !m_Transform )
    {
    itkExceptionMacro(<< "Transform not set");
    }

  if ( !m_Interpolator )
    {
    itkExceptionMacro(<< "Interpolator not set");
    }

  // Connect input image to interpolator
  m_Interpolator->SetInputImage( this->GetInput() );

  // Test for a BSpline interpolator
  BSplineInterpolatorType *testPtr =
    dynamic_cast< BSplineInterpolatorType * > ( m_Interpolator.GetPointer() );
  if ( testPtr )
    {
    m_InterpolatorIsBSpline = true;
    m_BSplineInterpolator = testPtr;
    m_BSplineInterpolator->SetNumberOfThreads( this->GetNumberOfThreads() );
    }
  else
    {
    m_InterpolatorIsBSpline = false;
    }
}

/**
 * Set up state of filter after multi-threading.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::AfterThreadedGenerateData()
{
  // Disconnect input image from the interpolator
  m_Interpolator->SetInputImage(NULL);
}

/**
 * ThreadedGenerateData
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType threadId)
{
  // Check whether the input or the output is a
  // SpecialCoordinatesImage.  If either are, then we cannot use the
  // fast path since index mapping will definately not be linear.
  typedef SpecialCoordinatesImage< PixelType, ImageDimension >
  OutputSpecialCoordinatesImageType;
  typedef SpecialCoordinatesImage< InputPixelType, InputImageDimension >
  InputSpecialCoordinatesImageType;

  if ( dynamic_cast< const InputSpecialCoordinatesImageType * >( this->GetInput() )
       || dynamic_cast< const OutputSpecialCoordinatesImageType * >
       ( this->GetOutput() ) )
    {
    this->NonlinearThreadedGenerateData(outputRegionForThread, threadId);
    return;
    }

  // Check whether we can use a fast path for resampling. Fast path
  // can be used if the transformation is linear. Transform respond
  // to the IsLinear() call.
  if ( m_Transform->IsLinear() )
    {
    this->LinearThreadedGenerateData(outputRegionForThread, threadId);
    return;
    }

  // Otherwise, we use the normal method where the transform is called
  // for computing the transformation of every point.
  this->NonlinearThreadedGenerateData(outputRegionForThread, threadId);
}

/**
 * NonlinearThreadedGenerateData
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::NonlinearThreadedGenerateData(const OutputImageRegionType &
                                outputRegionForThread,
                                ThreadIdType threadId)
{
  // Get the output pointers
  OutputImagePointer outputPtr = this->GetOutput();

  // Get ths input pointers
  InputImageConstPointer inputPtr = this->GetInput();

  // Create an iterator that will walk the output region for this thread.
  typedef ImageRegionIteratorWithIndex< TOutputImage > OutputIterator;
  OutputIterator outIt(outputPtr, outputRegionForThread);

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint;         // Coordinates of current output pixel
  PointType inputPoint;          // Coordinates of current input pixel

  ContinuousInputIndexType inputIndex;

  // Support for progress methods/callbacks
  ProgressReporter progress( this,
                             threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelType minValue =  NumericTraits< PixelType >::NonpositiveMin();
  const PixelType maxValue =  NumericTraits< PixelType >::max();

  typedef typename InterpolatorType::OutputType OutputType;
  const OutputType minOutputValue = static_cast< OutputType >( minValue );
  const OutputType maxOutputValue = static_cast< OutputType >( maxValue );

  // Walk the output region
  outIt.GoToBegin();

  while ( !outIt.IsAtEnd() )
    {
    // Determine the index of the current output pixel
    outputPtr->TransformIndexToPhysicalPoint(outIt.GetIndex(), outputPoint);

    // Compute corresponding input pixel position
    inputPoint = this->m_Transform->TransformPoint(outputPoint);
    inputPtr->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndex);

    // Evaluate input at right position and copy to the output
    if ( m_Interpolator->IsInsideBuffer(inputIndex) )
      {
      PixelType        pixval;
      OutputType       value;
      if ( m_InterpolatorIsBSpline )
        {
        value = m_BSplineInterpolator
                 ->EvaluateAtContinuousIndex(inputIndex, threadId);
        }
      else
        {
        value = m_Interpolator ->EvaluateAtContinuousIndex(inputIndex);
        }
      // Check boundaries and assign
      if ( value < minOutputValue )
        {
        pixval = minValue;
        }
      else if ( value > maxOutputValue )
        {
        pixval = maxValue;
        }
      else
        {
        pixval = static_cast< PixelType >( value );
        }
      outIt.Set(pixval);
      }
    else
      {
      outIt.Set(m_DefaultPixelValue); // default background value
      }

    progress.CompletedPixel();
    ++outIt;
    }

  return;
}

/**
 * LinearThreadedGenerateData
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::LinearThreadedGenerateData(const OutputImageRegionType &
                             outputRegionForThread,
                             ThreadIdType threadId)
{
  // Get the output pointers
  OutputImagePointer outputPtr = this->GetOutput();

  // Get ths input pointers
  InputImageConstPointer inputPtr = this->GetInput();

  // Create an iterator that will walk the output region for this thread.
  typedef ImageLinearIteratorWithIndex< TOutputImage > OutputIterator;

  OutputIterator outIt(outputPtr, outputRegionForThread);
  outIt.SetDirection(0);

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint;         // Coordinates of current output pixel
  PointType inputPoint;          // Coordinates of current input pixel
  PointType tmpOutputPoint;
  PointType tmpInputPoint;

  ContinuousInputIndexType inputIndex;
  ContinuousInputIndexType tmpInputIndex;

  typedef typename PointType::VectorType VectorType;
  VectorType delta;          // delta in input continuous index coordinate frame

  IndexType  index;

  // Support for progress methods/callbacks
  ProgressReporter progress( this,
                             threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  typedef typename InterpolatorType::OutputType OutputType;

  // Cache information from the superclass
  PixelType defaultValue = this->GetDefaultPixelValue();

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelType minValue =  NumericTraits< PixelType >::NonpositiveMin();
  const PixelType maxValue =  NumericTraits< PixelType >::max();

  const OutputType minOutputValue = static_cast< OutputType >( minValue );
  const OutputType maxOutputValue = static_cast< OutputType >( maxValue );

  // Determine the position of the first pixel in the scanline
  index = outIt.GetIndex();
  outputPtr->TransformIndexToPhysicalPoint(index, outputPoint);

  // Compute corresponding input pixel position
  inputPoint = this->m_Transform->TransformPoint(outputPoint);
  inputPtr->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndex);

  // As we walk across a scan line in the output image, we trace
  // an oriented/scaled/translated line in the input image.  Cache
  // the delta along this line in continuous index space of the input
  // image. This allows us to use vector addition to model the
  // transformation.
  //
  // To find delta, we take two pixels adjacent in a scanline
  // and determine the continuous indices of these pixels when
  // mapped to the input coordinate frame. We use the difference
  // between these two continuous indices as the delta to apply
  // to an index to trace line in the input image as we move
  // across the scanline of the output image.
  //
  // We determine delta in this manner so that Images
  // are both handled properly (taking into account the direction cosines).
  //
  ++index[0];
  outputPtr->TransformIndexToPhysicalPoint(index, tmpOutputPoint);
  tmpInputPoint = this->m_Transform->TransformPoint(tmpOutputPoint);
  inputPtr->TransformPhysicalPointToContinuousIndex(tmpInputPoint,
                                                    tmpInputIndex);
  delta = tmpInputIndex - inputIndex;

  while ( !outIt.IsAtEnd() )
    {
    // Determine the continuous index of the first pixel of output
    // scanline when mapped to the input coordinate frame.
    //

    // First get the position of the pixel in the output coordinate frame
    index = outIt.GetIndex();
    outputPtr->TransformIndexToPhysicalPoint(index, outputPoint);

    // Compute corresponding input pixel continuous index, this index
    // will incremented in the scanline loop
    inputPoint = this->m_Transform->TransformPoint(outputPoint);
    inputPtr->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndex);

    while ( !outIt.IsAtEndOfLine() )
      {
      // Evaluate input at right position and copy to the output
      if ( m_Interpolator->IsInsideBuffer(inputIndex) )
        {
        PixelType  pixval;
        OutputType value;
        if ( m_InterpolatorIsBSpline )
          {
          value = m_BSplineInterpolator->EvaluateAtContinuousIndex(
            inputIndex,
            threadId);
          }
        else
          {
          value = m_Interpolator->EvaluateAtContinuousIndex(inputIndex);
          }
        //Check for value min/max
        if ( value <  minOutputValue )
          {
          pixval = minValue;
          }
        else if ( value > maxOutputValue )
          {
          pixval = maxValue;
          }
        else
          {
          pixval = static_cast< PixelType >( value );
          }
        outIt.Set(pixval);
        }
      else
        {
        outIt.Set(defaultValue); // default background value
        }

      progress.CompletedPixel();
      ++outIt;
      inputIndex += delta;
      }
    outIt.NextLine();
    } //while( !outIt.IsAtEnd() )

  return;
}

/**
 * Inform pipeline of necessary input image region
 *
 * Determining the actual input region is non-trivial, especially
 * when we cannot assume anything about the transform being used.
 * So we do the easy thing and request the entire input image.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::GenerateInputRequestedRegion()
{
  // call the superclass's implementation of this method
  Superclass::GenerateInputRequestedRegion();

  if ( !this->GetInput() )
    {
    return;
    }

  // get pointers to the input and output
  InputImagePointer inputPtr  =
    const_cast< TInputImage * >( this->GetInput() );

  // Request the entire input image
  inputPtr->SetRequestedRegionToLargestPossibleRegion();

  return;
}

/**
 * Set the smart pointer to the reference image that will provide
 * the grid parameters for the output image.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
const typename ResampleImageFilter< TInputImage,
                                    TOutputImage,
                                    TInterpolatorPrecisionType >
::OutputImageType *
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::GetReferenceImage() const
{
  Self *                 surrogate = const_cast< Self * >( this );
  const OutputImageType *referenceImage =
    static_cast< const OutputImageType * >( surrogate->ProcessObject::GetInput(1) );

  return referenceImage;
}

/**
 * Set the smart pointer to the reference image that will provide
 * the grid parameters for the output image.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::SetReferenceImage(const TOutputImage *image)
{
  itkDebugMacro("setting input ReferenceImage to " << image);
  if ( image != static_cast< const TOutputImage * >( this->ProcessObject::GetInput(1) ) )
    {
    this->ProcessObject::SetNthInput( 1, const_cast< TOutputImage * >( image ) );
    this->Modified();
    }
}

/**
 * Inform pipeline of required output region
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::GenerateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();

  // get pointers to the input and output
  OutputImagePointer outputPtr = this->GetOutput();
  if ( !outputPtr )
    {
    return;
    }

  const OutputImageType *referenceImage = this->GetReferenceImage();

  // Set the size of the output region
  if ( m_UseReferenceImage && referenceImage )
    {
    outputPtr->SetLargestPossibleRegion(
      referenceImage->GetLargestPossibleRegion() );
    }
  else
    {
    typename TOutputImage::RegionType outputLargestPossibleRegion;
    outputLargestPossibleRegion.SetSize(m_Size);
    outputLargestPossibleRegion.SetIndex(m_OutputStartIndex);
    outputPtr->SetLargestPossibleRegion(outputLargestPossibleRegion);
    }

  // Set spacing and origin
  if ( m_UseReferenceImage && referenceImage )
    {
    outputPtr->SetSpacing( referenceImage->GetSpacing() );
    outputPtr->SetOrigin( referenceImage->GetOrigin() );
    outputPtr->SetDirection( referenceImage->GetDirection() );
    }
  else
    {
    outputPtr->SetSpacing(m_OutputSpacing);
    outputPtr->SetOrigin(m_OutputOrigin);
    outputPtr->SetDirection(m_OutputDirection);
    }

  return;
}

/**
 * Verify if any of the components has been modified.
 */
template< class TInputImage,
          class TOutputImage,
          class TInterpolatorPrecisionType >
unsigned long
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType >
::GetMTime(void) const
{
  unsigned long latestTime = Object::GetMTime();

  if ( m_Transform )
    {
    if ( latestTime < m_Transform->GetMTime() )
      {
      latestTime = m_Transform->GetMTime();
      }
    }

  if ( m_Interpolator )
    {
    if ( latestTime < m_Interpolator->GetMTime() )
      {
      latestTime = m_Interpolator->GetMTime();
      }
    }

  return latestTime;
}
} // end namespace itk

#endif
