# Specification: Automated Docker Build and Publish

## Overview
This track implements a GitHub Actions workflow (`docker.yml`) to automatically build multi-architecture Docker images for the `side-eye-host` agent and publish them to both Docker Hub and GitHub Container Registry (GHCR). This ensures that containerized versions of the host agent are readily available for users on various hardware platforms.

## Functional Requirements
1.  **Workflow Trigger:**
    -   Trigger the workflow on `push` of tags matching `v*.*.*`.
    -   Include `workflow_dispatch` for manual testing with a `fake_tag` input.
2.  **Multi-Architecture Support:**
    -   Build images for `linux/amd64`, `linux/arm/v7`, and `linux/arm64`.
    -   Use `docker/setup-qemu-action` and `docker/setup-buildx-action` for cross-platform builds.
3.  **Registry Publishing:**
    -   Publish to Docker Hub as `nicholaswilde/side-eye-host`.
    -   Publish to GHCR as `ghcr.io/nicholaswilde/side-eye-host`.
4.  **Metadata and Tagging:**
    -   Use `docker/metadata-action` to generate semantic versioning tags (e.g., `1.2.3`, `1.2`, `latest`).
    -   Support `fake_tag` for manual runs.
5.  **Build Optimization:**
    -   Implement the "build by digest" and "merge manifest" pattern from the template to handle multi-platform builds efficiently in parallel.
    -   Utilize GitHub Actions cache (`type=gha`) for faster builds.

## Non-Functional Requirements
-   **Security:** Use repository secrets for Docker Hub credentials (`DOCKERHUB_USERNAME`, `DOCKERHUB_TOKEN`).
-   **Efficiency:** parallelize architecture builds to reduce overall workflow duration.

## Acceptance Criteria
- [ ] A `.github/workflows/docker.yml` file exists.
- [ ] The workflow successfully builds images for all three target architectures.
- [ ] Images are successfully pushed to Docker Hub and GHCR upon tag push.
- [ ] Manifest lists (multi-arch images) are correctly created and verified.

## Out of Scope
-   Automated triggering of the `release.yml` workflow (they remain independent for now).
-   Dockerization of the firmware build process.
