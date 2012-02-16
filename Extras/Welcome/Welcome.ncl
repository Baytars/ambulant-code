<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
	Ambulant Welcome document tranlated into NCL
	needs adapted version of ginga installed, call:
	ginga - -disable-gfx  - -ncl Welcome.ncl - -mode 240x270
-->
<ncl id="syncEx" xmlns="http://www.ncl.org.br/NCL3.0/EDTVProfile">
  <head>
    <regionBase>
      <region id="backgroundReg" width="240" height="270" zIndex="1"/>
      <region id="screenReg" width="100%" height="100%" zIndex="2">
	<region id="titleReg" width="200" height="43" left="36" top="26" z-index="2"/>
	<region id="subtitleReg" width="193" height="19" left="35" top="68" z-index="2"/>
	<region id="line1Reg" width="116" height="30" left="20" top="95" z-index="2"/>
	<region id="line2Reg" width="143" height="30" left="60" top="130" z-index="2"/>
	<region id="line3Reg" width="167" height="26" left="70" top="164" z-index="2"/>
	<region id="infoReg" width="200" height="60" left="30" top="204" z-index="2"/>
      </region>
    </regionBase>
    <descriptorBase>
      <descriptor id="audioDesc"/>
      <descriptor id="backgroundDesc" region="backgroundReg"/>
      <descriptor id="titleDesc" region="titleReg"/>
      <descriptor id="subtitleDesc" region="subtitleReg"/>
      <descriptor id="line1Desc" region="line1Reg"/>
      <descriptor id="line2Desc" region="line2Reg"/>
      <descriptor id="line3Desc" region="line3Reg"/>
      <descriptor id="infoDesc" region="infoReg"/>
      </descriptorBase>
      <connectorBase>
	<causalConnector id="onBeginStartDelay">
	  <connectorParam name="delay"/>				
	  <simpleCondition role="onBegin"/>
	  <simpleAction role="start" delay="$delay" max="unbounded" qualifier="par"/>
	</causalConnector>
      </connectorBase>
  </head>

  <body>
    <port id="entry" component="background"/>

    <media id="music" src="data/AmbulantTune.mp3" descriptor="audioDesc" begin="0s"/>
    <media id="background" src="data/background.png" descriptor="backgroundDesc"/>
    <media id="title" src="data/ambulant.png" descriptor="titleDesc" />
    <media id="subtitle" src="data/extensible.png" descriptor="subtitleDesc" />
    <media id="line1" src="data/fast.png" descriptor="line1Desc" />
    <media id="line2" src="data/portable.png" descriptor="line2Desc" />
    <media id="line3" src="data/opensource.png" descriptor="line3Desc" />
    <media id="info" src="data/visit.png" descriptor="infoDesc" />

    <link xconnector="onBeginStartDelay">
      <bind component="background" role="onBegin"/>
      <bind component="music" role="start">
	<bindParam name="delay" value="0s"/>
      </bind>
      <bind component="title" role="start" >
	<bindParam name="delay" value="0s"/>
      </bind>
      <bind component="subtitle" role="start" >
	<bindParam name="delay" value="2s"/>
      </bind>
      <bind component="line1" role="start" >
	<bindParam name="delay" value="2.5s"/>
      </bind>
      <bind component="line2" role="start" >
	<bindParam name="delay" value="3s"/>
      </bind>
      <bind component="line3" role="start" >
	<bindParam name="delay" value="3.5s"/>
      </bind>
      <bind component="info" role="start" >
	<bindParam name="delay" value="4s"/>
      </bind>
    </link>
  </body>
</ncl>