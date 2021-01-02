# How to release a new blogc version

These are some rough steps required to produce a blogc release.


## Define version number

- Read repository logs:
  ```
  $ git log v0.18.0..HEAD  # replace with whatever latest release
  ```
- Follow this criteria:
  - Only bug fixes: Bump micro version.
  - Something new: Bump minor version, set micro to `0`.
  - We have no plans to change major version so far. Keep it `0`.


## Prepare release

- Write blog post (repository is https://github.com/blogc/blogc.rgm.io/)
  ```
  $ cp content/news/blogc-0.18.0.txt content/news/blogc-0.19.0.txt  # replace versions
  $ vim blogcfile  # change LATEST_RELEASE, add new post file to the end of [posts]
  $ vim content/news/blogc-0.19.0.txt  # write the content, based on what changed. put some placeholder DATE
  $ blogc-make runserver
  ```
- Copy manpage sources from main repository (https://github.com/blogc/blogc):
  ```
  $ cp -r ../blogc/man/* man/
  $ make -C man
  ```
  ronn output is not idempotent. It may rewrite HTML for manpages that are unchanged. Make sure to discard these changes somehow with git.
- Write GitHub release draft. Content is similar to blog post. No need to include lots of links. Always append this to the end of text:
  ```
  Please download the custom tarballs, not the files generated automatically by GitHub, they are garbage.
  ```


## Bump version

- Make sure that all the code is in place.
- Edit `blogc.spec.in`. Check if dependencies and packaging are still correct.
  - If something needs update, do it. Then commit, push and wait for [CI](https://github.com/blogc/blogc/actions). After successful build, grab a `.src.rpm` from https://distfiles.rgm.io/blogc/LATEST/, upload to a test [Copr](https://copr.fedorainfracloud.org/), wait until it builds and follow also next step.
  - If everything is ok, add a changelog line to the top of the `%changelog` section.
- Update debian version:
  - If something needs update, do it. Then commit, push and wait for [CI](https://github.com/blogc/blogc/actions).
  - Run `dch`:
    ```
    $ dch --distribution unstable --newversion 0.19.0-1~0upstream "Upstream Release"
    ```
- Commit, push and wait for [CI](https://github.com/blogc/blogc/actions).
- Create signed tag:
  ```
  $ git tag -s -m 'blogc-0.19.0' v0.19.0  # replace with whatever version you defined before
  ```
  This requires my (rafaelmartins) personal GPG key. If someone else needs to publish a release, please announce the new key in the blog post and GitHub release.
- Push signed tag:
  ```
  $ git push origin v0.19.0  # replace with whatever version you defined before
  ```
- Wait for [CI](https://github.com/blogc/blogc/actions) to build the release files and push them to https://distfiles.rgm.io/blogc/.


## Releasing new version

- Download new release files and sign them:
  ```
  $ ./download_release.py
  ```
  This requires my (rafaelmartins) personal GPG key to sign files. If someone else needs to publish a release, please edit the script to add new GPG key and announce it in the blog post and GitHub release.
- Grab files from `releases/0.19.0` (replace version) and upload to GitHub release draft, including `.asc` files. The `.src.rpm` and tarballs with debs can be omited.
- Publish release on GitHub.
- Fix blog post DATE, commit and push.


## Maintain downstream packages
- Upload `.src.rpm` file to the [official Copr](https://copr.fedorainfracloud.org/coprs/rafaelmartins/blogc/). The Copr is owned by rafaelmartins, there's no concept of organizations. I believe that an user could request publish access here: https://copr.fedorainfracloud.org/coprs/rafaelmartins/blogc/permissions/
- Prepare and submit [termux-packages](https://github.com/termux/termux-packages) pull request.
  - Edit `packages/blogc/build.sh`. Update version.
  - Try to build package (it will fail):
    ```
    $ ./scripts/run-docker.sh ./build-package.sh blogc
    ```
  - Get new SHA256 hash from last command and update `packages/blogc/build.sh`.
  - Build package:
    ```
    $ ./scripts/run-docker.sh ./build-package.sh blogc
    ```
  - If everything builds fine, commit and create pull request.

## Celebrate

    \o/
