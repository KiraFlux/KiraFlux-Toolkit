# Changelog

All notable changes to the KiraFlux Toolkit from version 0.3.0 to 0.3.1 are documented in this file.

## [0.3.1] – 2026-04-03

### Changed

- **Configuration structures now aggregate** – `ArduinoIIC::BusConfig`, `ArduinoSPI::BusConfig` and `ArduinoSPI::NodeConfig` are now true aggregates (no user‑defined constructors). Static factory methods `create()` replace the previous constructors. This enables designated initializers and improves constexpr usage. Existing code must be updated to call `Config::create(...)` instead of direct construction. ([#146](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/146))

### Fixed

- **ExponentialFilter initial transient** – The filter no longer starts from zero; the first input value is returned unchanged, eliminating an unwanted initial transient. ([#149](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/149))
- **MizLangBridge::poll() inverted condition** – Fixed a logic error where a successful read was treated as failure, causing incorrect error returns. ([#148](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/148))
- **ArithmeticPositiveOnlyAdjuster missing return** – The `adjustImpl` function now correctly returns the clamped value instead of falling through. ([#147](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/147))
- **UI::Page::link() constexpr and nodiscard** – The `link()` method is now `constexpr` and marked `[[nodiscard]]` to prevent accidental discarding of the returned widget reference. ([#145](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/145))

---

[0.3.1]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.1