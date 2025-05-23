<img width="150" height="150" align="left" style="float: left; margin: 0 10px 0 0;" alt="Cutter logo" src="https://raw.githubusercontent.com/rizinorg/cutter/dev/src/img/cutter.svg?sanitize=true">

# Cutter

Cutter is a free and open-source reverse engineering platform powered by [rizin](https://github.com/rizinorg/rizin). It aims at being an advanced and customizable reverse engineering platform while keeping the user experience in mind. Cutter is created by reverse engineers for reverse engineers.  

[![Cutter CI](https://github.com/rizinorg/cutter/workflows/Cutter%20CI/badge.svg)](https://github.com/rizinorg/cutter/actions?query=workflow%3A%22Cutter+CI%22)
[![Build status](https://ci.appveyor.com/api/projects/status/tn7kttv55b8wf799/branch/dev?svg=true)](https://ci.appveyor.com/project/rizinorg/cutter/branch/dev)

![Screenshot](https://raw.githubusercontent.com/rizinorg/cutter/dev/docs/source/images/screenshot.png)

## Learn more at [cutter.re](https://cutter.re).

## Getting Cutter
### Download

Cutter release binaries for all major platforms (Linux, macOS, Windows) can be downloaded from [GitHub Releases](https://github.com/rizinorg/cutter/releases).

- **Linux**: If your distribution provides it, check for `cutter` package in your package manager (or `cutter-re` / `rz-cutter`). If not available there, we have setup repositories in [OBS](https://openbuildservice.org/) for some common distributions. Look at [https://software.opensuse.org/package/cutter-re](https://software.opensuse.org/download/package?package=cutter-re&project=home%3ARizinOrg) and follow the instructions there. Otherwise download the `.AppImage` file from our release, make it executable and run as below or use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher).

    `chmod +x Cutter*.AppImage; ./Cutter*.AppImage`
- **macOS**: Download the `.dmg` file or use [Homebrew Cask](https://github.com/Homebrew/homebrew-cask):

    `brew install --cask cutter`
- **Windows**: Download the `.zip` archive, or use either [Chocolatey](https://chocolatey.org) or [Scoop](https://scoop.sh/):

    `choco install cutter`
    
    `scoop bucket add extras` followed by `scoop install cutter`

### Build from sources

To build Cutter from sources, please check the [Building Docs](https://cutter.re/docs/building.html).

### Docker image

To deploy *cutter* using a pre-built `Dockerfile`, it's possible to use the [provided configuration](docker). The corresponding `README.md` file also contains instructions on how to get started using the docker image with minimal effort.

## Documentation

### [User Guide](https://cutter.re/docs/user-docs.html)

### [Contribution Guidelines](https://cutter.re/docs/contributing.html)

### [Developers Docs](https://cutter.re/docs/contributing/code.html)

## Plugins
Cutter supports both Python and Native C++ plugins.

Our community has built many plugins and useful scripts for Cutter such as the native integration of [Ghidra decompiler](https://github.com/rizinorg/rz-ghidra) or the plugin to visualize DynamoRIO code coverage. You can find a list of cutter plugins linked below. Feel free to extend it with your own plugins and scripts for Cutter.

**[Official & Community Plugins](https://github.com/rizinorg/cutter-plugins)**

**[Plugins Development Guide](https://cutter.re/docs/plugins.html)**

## Getting Help

Please use the following channels to ask for help from Cutter developers and community:

- **Telegram:** https://t.me/cutter_re
- **Mattermost:** https://im.rizin.re
- **IRC:** #cutter on https://web.libera.chat/
- **Twitter:** [@cutter_re](https://twitter.com/cutter_re)
