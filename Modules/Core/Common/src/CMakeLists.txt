set(ITK-Common_SRCS
itkLoggerOutput.cxx
itkProgressAccumulator.cxx
itkNumericTraits.cxx
itkMutexLock.cxx
itkHexahedronCellTopology.cxx
itkIndent.cxx
itkEventObject.cxx
itkFileOutputWindow.cxx
itkSimpleFilterWatcher.cxx
itkKernelFunction.cxx
itkGaussianKernelFunction.cxx
itkNumericTraitsVectorPixel.cxx
itkNumericTraitsVariableLengthVectorPixel.cxx
itkObject.cxx
itkDynamicLoader.cxx
itkQuadrilateralCellTopology.cxx
itkIterationReporter.cxx
itkMemoryProbe.cxx
itkTextOutput.cxx
itkNumericTraitsTensorPixel2.cxx
itkNumericTraitsFixedArrayPixel2.cxx
itkConditionVariable.cxx
itkProcessObject.cxx
itkBarrier.cxx
itkSpatialOrientationAdapter.cxx
itkRealTimeInterval.cxx
itkOctreeNode.cxx
itkNumericTraitsFixedArrayPixel.cxx
itkMultiThreader.cxx
itkNumericTraitsArrayPixel.cxx
itkMetaDataDictionary.cxx
itkDataObject.cxx
itkThreadLogger.cxx
itkNumericTraitsTensorPixel.cxx
itkCommand.cxx
itk_hashtable.cxx
itkNumericTraitsPointPixel.cxx
itkLightObject.cxx
itkStdStreamLogOutput.cxx
itkLightProcessObject.cxx
itkRegion.cxx
itkFastMutexLock.cxx
itkVersion.cxx
itkNumericTraitsRGBAPixel.cxx
itkRealTimeClock.cxx
itkMetaDataObjectBase.cxx
itkCovariantVector.cxx
itkMemoryUsageObserver.cxx
itkMersenneTwisterRandomVariateGenerator.cxx
itkLoggerBase.cxx
itkNumericTraitsCovariantVectorPixel.cxx
itkProgressReporter.cxx
itkExceptionObject.cxx
itkMultipleLogOutput.cxx
itkQuadraticTriangleCellTopology.cxx
itkTimeProbesCollectorBase.cxx
itkSmapsFileParser.cxx
itkTriangleCellTopology.cxx
itkVector.cxx
itkRealTimeStamp.cxx
itkDirectory.cxx
itkLoggerManager.cxx
itkTimeProbe.cxx
itkNumericTraitsRGBPixel.cxx
itkTimeStamp.cxx
itkTetrahedronCellTopology.cxx
itkObjectFactoryBase.cxx
itkFloatingPointExceptions.cxx
itkOutputWindow.cxx
itkSimpleFastMutexLock.cxx
itkNumericTraitsDiffusionTensor3DPixel.cxx
itkEquivalencyTable.cxx
itkXMLFileOutputWindow.cxx
itkStoppingCriterionBase.cxx
)

if(WIN32)
   set(ITK-Common_SRCS ${ITK-Common_SRCS};itkWin32OutputWindow.cxx;itkSmartPointerForwardReferenceProcessObject.cxx)
endif(WIN32)

### generating libraries
add_library( ITK-Common ${ITK_LIBRARY_BUILD_TYPE} ${ITK-Common_SRCS})

target_link_libraries(ITK-Common itksys ITK-VNLInstantiation)

if(UNIX)
  target_link_libraries(ITK-Common ${CMAKE_THREAD_LIBS} ${CMAKE_DL_LIBS} -lm)
endif(UNIX)

itk_module_target(ITK-Common)
