#!/bin/bash

HUNTER_CONFIGURATION_TYPES=Release

UNITY_NATIVEX_BUILD_ARGS=\
(
	"HUNTER_CONFIGURATION_TYPES=Release"
	"BUILD_QT=${BUILD_QT}"
)

function rename_tab
{
	echo -ne "\033]0;$1:$2\007"
}