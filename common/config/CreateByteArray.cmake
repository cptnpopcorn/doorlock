function(create_byte_array HEXSTR ARRNAME OUTVAR)
    string(LENGTH "${HEXSTR}" _len)
    if(NOT _len EQUAL 32)
        message(FATAL_ERROR
            "AES key '${ARRNAME}' must be exactly 32 hex characters (16 bytes)."
        )
    endif()

    # Generate comma-separated bytes
    set(_bytes "")
    foreach(i RANGE 0 15)
        math(EXPR idx "2*${i}")
        string(SUBSTRING "${HEXSTR}" ${idx} 2 _pair)
        string(APPEND _bytes "0x${_pair}, ")
    endforeach()

    # Produce C++ constexpr array declaration text
    set(${OUTVAR} "constexpr std::array<std::uint8_t, 16> ${ARRNAME} = {{ ${_bytes} }};" PARENT_SCOPE)
endfunction()