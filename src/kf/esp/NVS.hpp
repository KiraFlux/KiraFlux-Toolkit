// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <nvs.h>
#include <nvs_flash.h>

#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct NvsError {

    enum Kind : char {
        InvalidLength,
        NotFound,
        NotEnoughSpace,
        BlobSizeMismatch,
        Unknown,
    } kind;

    [[nodiscard]] static constexpr auto fromEsp(const esp_err_t e) -> ResultErrorWrapper<NvsError> {
        switch (e) {
            case ESP_ERR_NVS_NOT_FOUND:
                return {NotFound};

            case ESP_ERR_NVS_INVALID_LENGTH:
                return {InvalidLength};

            case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
                return {NotEnoughSpace};

            case ESP_ERR_NVS_INVALID_HANDLE:
            case ESP_ERR_NVS_READ_ONLY:
            case ESP_ERR_NVS_INVALID_NAME:
            default:
                return {Unknown};
        }
    }
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
    explicit constexpr NVS(const char *nvs_namespace) noexcept :
        _namespace{nvs_namespace} {}

    /// @brief Get NVS Namespace name
    [[nodiscard]] constexpr const char *name() const noexcept {
        return _namespace;
    }

    /// @brief Get blob value
    /// @param key blob entry key
    /// @param buffer blob destination buffer
    [[nodiscard]] ResultType getBlob(const char *key, Slice<u8> buffer) noexcept {
        auto len = buffer.size();
        const auto result = wrap(nvs_get_blob(_handle.unwrap(), key, static_cast<void *>(buffer.data()), &len));

        if (len != buffer.size()) {
            return error(Error{Error::BlobSizeMismatch});
        }

        return result;
    }

    /// @brief Set blob value
    /// @param key blob entry key
    /// @param buffer blob source buffer
    [[nodiscard]] ResultType setBlob(const char *key, Slice<const u8> buffer) noexcept {
        return wrap(nvs_set_blob(_handle.unwrap(), key, buffer.data(), buffer.size()));
    }

    /// @brief Commit any pending writes
    [[nodiscard]] ResultType commit() noexcept {
        return wrap(nvs_commit(_handle.unwrap()));
    }

private:
    const char *_namespace;
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