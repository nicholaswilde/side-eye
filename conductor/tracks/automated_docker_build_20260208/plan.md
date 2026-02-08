# Implementation Plan: Automated Docker Build and Publish

## Phase 1: Workflow Scaffolding
- [x] Task: Create `.github/workflows/docker.yml` bbd09e0
    - [x] Define name, triggers (tags, workflow_dispatch), and environment variables.
    - [x] Implement the `build` job using the "by digest" strategy for `linux/amd64`, `linux/arm/v7`, and `linux/arm64`.
    - [x] Configure `docker/metadata-action` for image tagging.
    - [x] Add login steps for Docker Hub and GHCR.
    - [x] Add the `Export digest` and `Upload digest` steps.
- [x] Task: Implement the `merge` job bbd09e0
    - [x] Define the `merge` job that `needs` the `build` job.
    - [x] Implement `Download digests`, `Set up Docker Buildx`, and `Docker meta`.
    - [x] Implement `Create manifest list and push` using `docker buildx imagetools`.
- [x] Task: Conductor - User Manual Verification 'Workflow Scaffolding' (Protocol in workflow.md) 9f58e49

## Phase 2: Secrets and Validation
- [ ] Task: Verify Repository Secrets
    - [ ] Ensure `DOCKERHUB_USERNAME` and `DOCKERHUB_TOKEN` are ready for use (note: instructions for the user).
- [ ] Task: Perform Dry-Run/Manual Validation
    - [ ] Run the workflow manually using `workflow_dispatch` with a `fake_tag` (if permissions allow).
    - [ ] Verify that artifacts (digests) are correctly uploaded and the manifest creation step logic is sound.
- [ ] Task: Conductor - User Manual Verification 'Secrets and Validation' (Protocol in workflow.md)
