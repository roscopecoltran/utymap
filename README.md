<a href="https://play.google.com/store/apps/details?id=com.utymap.demo&utm_source=global_co&utm_medium=prtnr&utm_content=Mar2515&utm_campaign=PartBadge&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1"><img src="https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png" alt="" height="64" /></a>

[![experimental](https://img.shields.io/badge/stability-experimental-orange.svg?style=flat)](https://github.com/reinterpretcat/utymap)
[![Build Status](https://travis-ci.org/reinterpretcat/utymap.svg?branch=master)](https://travis-ci.org/reinterpretcat/utymap)
[![Coverity](https://scan.coverity.com/projects/10159/badge.svg)](https://scan.coverity.com/projects/reinterpretcat-utymap)
[![Release](https://img.shields.io/github/release/reinterpretcat/utymap.svg)](https://github.com/reinterpretcat/utymap/releases/latest)
[![Join the chat at https://gitter.im/reinterpretcat/utymap](https://badges.gitter.im/reinterpretcat/utymap.svg)](https://gitter.im/reinterpretcat/utymap?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

<h2> Description </h2>

UtyMap is a library which provides highly customizable API for procedural world generation based on real map data, e.g. OpenStreetMap, NaturalEarth. Core logic is written on C++11 and can be used on many platforms as it has no dependency to specific game engine or application framework. You can use it to create interactive world at different zoom levels, including globe and ground:

<img src="https://cloud.githubusercontent.com/assets/1611077/19216383/4c10f3ee-8db9-11e6-8877-b652aa846734.png" />

<h2> Project structure </h2>
Project consists of two sub-projects:
<ul>
    <li><b>core</b> contains essential logic written on C++11 to build library for constructing of map based apps: terrain/buildings/osm-objects mesh generators, mapcss parser, spatial geo index, etc. It does not depend on any 3D render framework or game engine. </li>
    <li><b>unity</b> contains examples written on C# which can be reused to build map oriented Unity3D apps using core library. It demonstrates basic use cases: globe zoom level rendering, 3D scene with all details.</li>
</ul>

<h2> Install </h2>
See instructions [here] (https://github.com/reinterpretcat/utymap/wiki#install).
