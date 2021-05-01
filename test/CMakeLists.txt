# To get verbose output: cmake --build build --target "test" -- ARGS='-V'

# By default, we run the spec tests only if python3 is available.
# To require the spec tests, compile with -DSPEC_TESTS=1

if (SPEC_TESTS)
  find_package(PythonInterp 3 REQUIRED)
else(SPEC_TESTS)
  find_package(PythonInterp 3)
endif(SPEC_TESTS)

if (CMARK_SHARED OR CMARK_STATIC)
  add_test(NAME api_test COMMAND api_test)
endif()

if (WIN32)
  file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/src WIN_DLL_DIR)
  set(NEWPATH "${WIN_DLL_DIR};$ENV{PATH}")
  string(REPLACE ";" "\\;" NEWPATH "${NEWPATH}")
  set_tests_properties(api_test PROPERTIES ENVIRONMENT "PATH=${NEWPATH}")
  set(ROUNDTRIP "${CMAKE_CURRENT_SOURCE_DIR}/roundtrip.bat")
else(WIN32)
  set(ROUNDTRIP "${CMAKE_CURRENT_SOURCE_DIR}/roundtrip.sh")
endif(WIN32)

IF (PYTHONINTERP_FOUND)

  if(XCODE)
    set(CMARK_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/../src/${CMAKE_BUILD_TYPE}")
  else()
    set(CMARK_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/../src")
  endif()

  add_test(html_normalization
    ${PYTHON_EXECUTABLE} "-m" "doctest"
    "${CMAKE_CURRENT_SOURCE_DIR}/normalize.py"
    )

  if (CMARK_SHARED)
    add_test(spectest_library
      ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/spec_tests.py" "--no-normalize" "--spec"
      "${CMAKE_CURRENT_SOURCE_DIR}/spec.txt" "--library-dir" "${CMARK_LIBRARY_DIR}"
      )

    add_test(pathological_tests_library
      ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/pathological_tests.py"
      "--library-dir" "${CMARK_LIBRARY_DIR}"
      )

    add_test(roundtriptest_library
      ${PYTHON_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/roundtrip_tests.py"
      "--spec" "${CMAKE_CURRENT_SOURCE_DIR}/spec.txt"
      "--library-dir" "${CMARK_LIBRARY_DIR}"
      )

    add_test(entity_library
      ${PYTHON_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/entity_tests.py"
      "--library-dir" "${CMARK_LIBRARY_DIR}"
      )
  endif()

  add_test(spectest_executable
    ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/spec_tests.py" "--no-normalize" "--spec" "${CMAKE_CURRENT_SOURCE_DIR}/spec.txt" "--program" "${CMARK_LIBRARY_DIR}/cmark"
    )

  add_test(smartpuncttest_executable
    ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/spec_tests.py" "--no-normalize" "--spec" "${CMAKE_CURRENT_SOURCE_DIR}/smart_punct.txt" "--program" "${CMARK_LIBRARY_DIR}/cmark --smart"
    )

  add_test(regressiontest_executable
    ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/spec_tests.py" "--no-normalize" "--spec"
    "${CMAKE_CURRENT_SOURCE_DIR}/regression.txt" "--program"
    "${CMARK_LIBRARY_DIR}/cmark"
    )

ELSE(PYTHONINTERP_FOUND)

  message("\n*** A python 3 interpreter is required to run the spec tests.\n")
  add_test(skipping_spectests
    echo "Skipping spec tests, because no python 3 interpreter is available.")

ENDIF(PYTHONINTERP_FOUND)

