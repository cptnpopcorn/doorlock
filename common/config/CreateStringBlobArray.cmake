function(create_string_blob_array BLOB_LITERAL ARRNAME OUTVAR)
    # Normalize CRLF
    string(REGEX REPLACE "\r" "" _blob "${BLOB_LITERAL}")

    # Pick a delimiter unlikely to appear inside certs
    set(_delim "CMK_KMC")

    # Detect delimiter conflict
    string(FIND "${_blob}" ")${_delim}" _found)
    if(NOT _found EQUAL -1)
        # If conflict: append a random suffix
        string(RANDOM LENGTH 6 ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ" _suffix)
        set(_delim "PEM_CMK_${_suffix}")
    endif()

    set(_decl
"constexpr auto ${ARRNAME} = to_u8_array(R\"${_delim}(
${_blob}
)${_delim}\");")

    set(${OUTVAR} "${_decl}" PARENT_SCOPE)
endfunction()