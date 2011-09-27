set(DOCUMENTATION "This module contains IO classes for reading and writing from
the <a href=\"http://www.itk.org/Wiki/MetaIO/Documentation\">MetaIO</a> format.
The MetaIO formats for objects includes a format for images (MetaImage).  A
MetaImage can either consist of a simple plain text header coupled with a data
file (usually .mhd + .raw) or the header inline with the data (usually .mha).")

itk_module(ITKIOMeta
  DEPENDS
    ITKMetaIO
    ITKIOBase
  TEST_DEPENDS
    ITKTestKernel
    ITKSmoothing
  DESCRIPTION
    "${DOCUMENTATION}"
)

# Extra test dependency of ITKSmoothing is caused by itkMetaStreamingIOTest.
