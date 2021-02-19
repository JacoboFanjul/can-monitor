#!/bin/sh

#docker login registry.gitlab.com
#docker buildx build -t "registry.gitlab.com/adeptness/source/adeptness-example-ms-c/test_can_monitor:0.0.0" --platform linux/amd64,linux/armhf,linux/arm64 --push .
docker login registry.bda.ikerlan.es
docker buildx build -t "registry.bda.ikerlan.es/adeptness/can_monitor:latest" --platform linux/amd64,linux/armhf,linux/arm64 --push .
#docker buildx build -t "registry.bda.ikerlan.es/adeptness/arm_test_can_monitor:0.0.0" --platform linux/armhf --push .

