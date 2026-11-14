# WY Saturator — GitHub Actions Mac build

Upload `CMakeLists.txt`, `Source/`, and `.github/` into the root of your GitHub repository (preserve folder structure). GitHub Actions will attempt to build a macOS universal VST3/AU and an **unsigned** installer.

Go to Actions > Build WY Saturator for macOS > Run workflow. If build succeeds, download the artifact from the workflow run page.

This is an untested source build, not a signed or notarized public release. JUCE is fetched during build; review its licensing for distribution. Do not publicly distribute unsigned packages as a production release.
