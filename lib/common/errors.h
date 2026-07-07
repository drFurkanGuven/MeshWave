#pragma once

/**
 * @file errors.h
 * @brief Unified error codes for LoRaLink.
 */

namespace loralink {

enum class Error : int {
    None            = 0,
    InvalidArgument = 1,
    BufferTooSmall  = 2,
    CrcMismatch     = 3,
    Timeout         = 4,
    RadioNotReady   = 5,
    RadioTxFailed   = 6,
    RadioRxFailed   = 7,
    EncodeFailed    = 8,
    DecodeFailed    = 9,
    DuplicatePacket = 10,
    RetryExhausted  = 11,
    StorageFull     = 12,
    StorageIoError  = 13,
    NotFound        = 14,
    NotSupported    = 15,
    WifiFailed      = 16,
    CryptoFailed    = 17,
    FragmentError   = 18,
    NotInitialized  = 19,
};

inline const char* errorToString(Error e) {
    switch (e) {
        case Error::None:            return "None";
        case Error::InvalidArgument: return "InvalidArgument";
        case Error::CrcMismatch:     return "CrcMismatch";
        case Error::Timeout:         return "Timeout";
        case Error::RadioNotReady:   return "RadioNotReady";
        case Error::RetryExhausted:  return "RetryExhausted";
        default:                     return "Unknown";
    }
}

} // namespace loralink
