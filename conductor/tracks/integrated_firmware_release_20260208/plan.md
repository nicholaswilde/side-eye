# Implementation Plan: Integrated Firmware and Host Release

## Phase 1: Workflow Integration [checkpoint: 5d79b1a]
- [x] Task: Create a new `build-firmware` job in `release.yml`
    - [x] Define the job to run on `ubuntu-latest`.
    - [x] Install PlatformIO and dependencies (Python, etc.).
    - [x] Configure `pio run` command to build the default environment.
    - [x] Inject the version tag into the build process (likely via `PIO_BUILD_FLAGS` or an env var used by `extra_scripts`).
- [x] Task: Implement Artifact Zipping
    - [x] Create a step to locate `firmware.bin`, `bootloader.bin`, and `partitions.bin` in `.pio/build/`.
    - [x] Zip these files into `side-eye-<tag_name>-firmware.zip`.
    - [x] Upload the zip as a workflow artifact to be consumed by the release job.
- [x] Task: Update `create-release` job
    - [x] Ensure the release creation step outputs the `upload_url` or simply exists before artifact upload.
    - [x] (Note: `softprops/action-gh-release` handles appending assets well, so this might just be a dependency configuration).
- [x] Task: Conductor - User Manual Verification 'Workflow Integration' (Protocol in workflow.md) 5d79b1a

## Phase 2: Asset Upload & Finalization
- [ ] Task: Integrate Firmware Asset Upload
    - [ ] Add a step (or separate job) to download the firmware artifact.
    - [ ] Use `gh release upload` or `softprops/action-gh-release` to attach the firmware zip to the created draft release.
- [ ] Task: Verify Integrated Workflow
    - [ ] Trigger a manual workflow run (using `workflow_dispatch` with a fake version) or a test tag.
    - [ ] Verify both host and firmware assets appear in the release.
- [ ] Task: Conductor - User Manual Verification 'Asset Upload & Finalization' (Protocol in workflow.md)
