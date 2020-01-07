FROM node:lts-buster

ARG PARTICLE_CLI_VERSION=latest

RUN apt update && apt install build-essential
RUN npm install -g --unsafe-perm particle-cli@$PARTICLE_CLI_VERSION
