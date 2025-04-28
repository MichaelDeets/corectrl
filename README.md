# CoreCtrl

[![CoreCtrl 1.0 overview video](https://gitlab.com/corectrl/corectrl/wikis/img/overview-1.0.png)](https://www.youtube.com/watch?v=6uchS6OiwiU)

> [!note]
> **CoreCtrl is in maintenance mode.** This means no new features or hardware support will be added, and development will focus solely on bug fixes and maintenance-related changes.
>
> It supports AMD GPUs up to the RX 9000 series, though some series may have limitations due to long-standing and unresolved driver or firmware issues (see [known issues wiki page](https://gitlab.com/corectrl/corectrl/-/wikis/Known-issues#amd-gpus)). Partial or full functionality on newer hardware is possible but not guaranteed. See [alternatives](README#alternatives) for applications with similar functionality.

**CoreCtrl** is a Free and Open Source GNU/Linux application that allows you to control with ease your computer hardware using application profiles. It aims to be flexible, comfortable and accessible to regular users.

There are already others GNU/Linux applications that allow you to control your hardware. *Some* of them are pretty good. *Most* of them are not built with regular users in mind and/or are focused on some specific hardware or features, so usually you end up with multiple control programs installed and running at the same time, each of them having its own specific configuration. Also, most of them do not respond to external events other that the hardware events they control so, if you want to change the behaviour of the system for a given period of time, let's say, during one specific program execution, you have to manually interact with each control program in order to change its behaviour, before and after that specific program execution.

All of this is perceived by regular users as a big burden or even a barrier that impedes them to migrate to GNU/Linux for some specific tasks (as gaming).

**CoreCtrl** aims to be a game changer in this particular field. You can use it to automatically configure your system when a program is launched (works for Windows applications too). It doesn't matter what the program is, a game, a 3D modelling application, a video editor or... even a compiler! It offers you full hardware control per application.

**CoreCtrl** works with Linux and Windows applications, has basic CPU controls and full AMD GPUs controls (for both old and new models up to RX 9000 series).

## Installation

### Distribution packages

> [!note]
> This list may contain unofficial distribution packages, maintained by other people not directly related with this protect. Please report any problems you find in these packages to them.

> [!warning]
> For security reasons, exercise extra caution with unofficial distribution packages. If you're unsure about them, you can either wait for your distribution to package CoreCtrl officially or [install it from the source code](https://gitlab.com/corectrl/corectrl/-/wikis/Installation). If you encounter anything malicious in any listed package, please open an issue so the list can be updated.

#### Arch Linux

    pacman -S corectrl

#### Fedora

    sudo dnf install corectrl

#### Gentoo

Add the [farmboy0](https://github.com/farmboy0/portage-overlay) overlay.

Then run:

    emerge --ask --verbose kde-misc/corectrl

#### openSUSE

Install the [corectrl](https://software.opensuse.org/download.html?project=home%3ADead_Mozay&package=corectrl) package from OBS.

#### Debian / Ubuntu

    sudo apt install corectrl

If you use an Ubuntu version that does not ship CoreCtrl officially, you can install it from the [`Ernst ppa-mesarc`](https://launchpad.net/~ernstp/+archive/ubuntu/mesarc) PPA.

> [!warning]
> This repository also hosts **release candidate and development versions** of many other packages. Notice that, by installing such packages, you can run into bugs that could break your system.

Most users may only want to install `corectrl` from this PPA. If so, create the file `/etc/apt/preferences.d/corectrl` with the following content:

    # Never prefer packages from the ernstp repository
    Package: *
    Pin: release o=LP-PPA-ernstp-mesarc
    Pin-Priority: 1

    # Allow upgrading only corectrl from LP-PPA-ernstp-mesarc
    Package: corectrl
    Pin: release o=LP-PPA-ernstp-mesarc
    Pin-Priority: 500

Then run:

    sudo apt install corectrl

#### Others

For other installation methods, see [Installation](https://gitlab.com/corectrl/corectrl/-/wikis/Installation).

## Setup

It's **strongly recommended** to [setup your system](https://gitlab.com/corectrl/corectrl/-/wikis/Setup) for better user experience and to unlock **hidden hardware features**.

## More info

Check the [Wiki](https://gitlab.com/corectrl/corectrl/wikis/home) for more useful info.

## Alternatives

Here is a non-exhaustive list of alternative applications with similar functionality:
- [LACT](https://github.com/ilya-zlobintsev/LACT)
- [TuxClocker](https://github.com/Lurkki14/tuxclocker)
