// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <nvs.h>
#include <nvs_flash.h>

#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/StringView.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/Representable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct NvsError : mixin::Representable<NvsError, StringView> {

    enum Kind : char {
        InvalidLength,
        NotFound,
        NotEnoughSpace,
        BlobSizeMismatch,
        Unknown,
    } kind;

    [[nodiscard]] static constexpr auto fromEsp(esp_err_t const e) -> ResultErrorWrapper<NvsError> {
        switch (e) {
            case ESP_ERR_NVS_NOT_FOUND:
                return {{.kind = NotFound}};

            case ESP_ERR_NVS_INVALID_LENGTH:
                return {{.kind = InvalidLength}};

            case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
                return {{.kind = NotEnoughSpace}};

            case ESP_ERR_NVS_INVALID_HANDLE:
            case ESP_ERR_NVS_READ_ONLY:
            case ESP_ERR_NVS_INVALID_NAME:
            default:
                return {{.kind = Unknown}};
        }
    }

private:
#define CASE_RETURN(__v) \
    case __v: return #__v

    KF_IMPL_REPRESENTABLE(NvsError, StringView);
    StringView reprImpl() const noexcept {

        switch (kind) {
            CASE_RETURN(NvsError::InvalidLength);
            CASE_RETURN(NvsError::NotFound);
            CASE_RETURN(NvsError::NotEnoughSpace);
            CASE_RETURN(NvsError::BlobSizeMismatch);
            default:
                CASE_RETURN(NvsError::Unknown);
        }
    }

#undef CASE_RETURN
};

}// namespace kf::internal

namespace kf::esp {

/// @brief NVS wrapper
struct NVS final :

    mixin::NonCopyable,
    mixin::Initable<NVS, Result<void, internal::NvsError>()>,
    mixin::Quitable<NVS>

{
    using Error = internal::NvsError;

    using ResultType = Result<void, Error>;

    /// @brief Construct NVS storage component
    explicit constexpr NVS(char const *nvs_namespace) noexcept :
        _namespace{nvs_namespace} {}

    /// @brief Get NVS Namespace name
    [[nodiscard]] constexpr char const *name() const noexcept {
        return _namespace;
    }

    /// @brief Get blob value
    /// @param key blob entry key
    /// @param buffer blob destination buffer
    [[nodiscard]] ResultType getBlob(char const *key, Slice<u8> buffer) noexcept {
        auto len = buffer.length();
        auto const result = wrap(nvs_get_blob(_handle.unwrap(), key, static_cast<void *>(buffer.data()), &len));

        if (len != buffer.length()) {
            return error(Error{.kind = Error::BlobSizeMismatch});
        }

        return result;
    }

    /// @brief Set blob value
    /// @param key blob entry key
    /// @param buffer blob source buffer
    [[nodiscard]] ResultType setBlob(char const *key, Slice<u8 const> buffer) noexcept {
        return wrap(nvs_set_blob(_handle.unwrap(), key, buffer.data(), buffer.length()));
    }

    /// @brief Commit any pending writes
    [[nodiscard]] ResultType commit() noexcept {
        return wrap(nvs_commit(_handle.unwrap()));
    }

private:
    char const *_namespace;
    TrivialOption<nvs_handle_t> _handle{none};

    [[nodiscard]] static ResultType wrap(esp_err_t e) noexcept {
        if (ESP_OK == e) {
            return ok();
        } else {
            return Error::fromEsp(e);
        }
    }

    KF_IMPL_INITABLE(NVS, Result<void, Error>());
    auto initImpl() noexcept -> Result<void, Error> {
        if (_handle.isSome()) {
            // Already initialised
            return ok();
        }

        esp_err_t e;
        nvs_handle_t handle;

        // Ensure NVS flash is initialised (idempotent)
        e = nvs_flash_init();
        if (e == ESP_ERR_NVS_NO_FREE_PAGES or e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated, need to erase and retry
            e = nvs_flash_erase();
            if (e != ESP_OK) {
                return Error::fromEsp(e);
            }
            e = nvs_flash_init();
        }

        if (e != ESP_OK) {
            return Error::fromEsp(e);
        }

        e = nvs_open(_namespace, NVS_READWRITE, &handle);
        if (ESP_OK != e) {
            return Error::fromEsp(e);
        }

        _handle = someTrivial(handle);
        return ok();
    }

    KF_IMPL_QUITABLE(NVS);
    void quitImpl() noexcept {
        if (_handle.isSome()) {
            nvs_close(_handle.unwrap());
            _handle.reset();
        }
    }
};

}// namespace kf::esp