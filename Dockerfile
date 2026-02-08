FROM rust:latest AS chef
# Install system dependencies
RUN apt-get update && apt-get install -y \
    libudev-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

RUN cargo install cargo-chef
WORKDIR /app

FROM chef AS planner
# Copy ONLY the host directory where the Rust project lives
COPY host/ .
RUN cargo chef prepare --recipe-path recipe.json

FROM chef AS builder
COPY --from=planner /app/recipe.json recipe.json
# Build dependencies - this is the caching Docker layer!
RUN cargo chef cook --release --recipe-path recipe.json
# Build application
COPY host/ .
RUN cargo build --release --bin side-eye-host

# Runtime stage
FROM gcr.io/distroless/cc-debian12 AS runtime
WORKDIR /app
# Copy the binary
COPY --from=builder /app/target/release/side-eye-host /app/side-eye-host
ENTRYPOINT ["/app/side-eye-host"]
