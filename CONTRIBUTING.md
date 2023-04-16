# Contributing
CoreCtrl welcomes your contributions.

## Build instructions
You can find the build instructions on the [wiki](https://gitlab.com/corectrl/corectrl/wikis/Installation).

## Issues writing guidelines
Please, follow these guidelines to write your issues. Reported issues not following this guidelines may be closed as invalid.

* One issue per report.
* Be precise and clear.
* Use the issue templates.
* Use good descriptive titles for new issues.

As first step, read the [FAQ](https://gitlab.com/corectrl/corectrl/wikis/FAQ) and [Known issues](https://gitlab.com/corectrl/corectrl/wikis/Known-issues). If your issue doesn't appear on them, search for similar issues on the issue tracker. If you find an already opened issue and you can provide new information, then add a new comment (please, do not write +1 o similar comments). If the issue is closed, open a new one and include a reference to the old one (for example, add 'Related to #<closed_issue_number>').

## Merge request
Before starting to work on a merge request, please follow these instructions:

1. Open an issue explaining the reason for the change, state that you want to work on it and wait for the developers response.
2. Create a merge request from a **new branch** (do not create the MR from the master branch) and link it with the issue. The merge request must be focused on only one feature (a translation, a sensor, a control, ...).
3. Follow the [coding style](#coding-style) rules and make sure your code integrates well into the application architecture.
4. Make sure that all unit tests pass when you are working on existing controls. If you are working on a new control, you must write tests for it.
5. Commit your work on atomic unit of changes. For example, if you are working on a new sensor that requires the refactoring of an existing function, make a commit with the function refactor first and then make another commit with the sensor implementation (including all the new sensor related stuff, like translations and new utility functions and their tests). If you are unsure of how to do it, check the commit history for plenty of examples of such commits.
6. Explain the changes introduced by non straightforward commits on their commit message.
7. During the merge request, be concise on your comments and make sure that you fully understand what you are stating on them. Take your time and, when needed, do your research before posting.

## Coding style
* Format your code with clang-format. You can do it with your editor or running `./format.sh` on the root project directory, then pick the formatted files that you are working on.
* Use [east const](https://mariusbancila.ro/blog/2018/11/23/join-the-east-const-revolution/).
* Use plain C++17 as much as possible. Don't use Qt functionality unless you have to.
