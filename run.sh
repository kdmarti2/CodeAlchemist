#!/bin/bash

echo "rewriting the provide JS samples"
dotnet bin/Main.dll rewrite /CodeAlchemist/config.json

echo "instrumenting the provide JS samples"
dotnet bin/Main.dll instrument /CodeAlchemist/config.json

echo "Starting to fuzz V8"
export RECORD=true
dotnet bin/Main.dll fuzz /CodeAlchemist/config.json
