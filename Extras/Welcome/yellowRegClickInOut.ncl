<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
     yellowRegClickInOut.ncl: very simple example to render a yellow rectangle
     on a gray background; when the yellow region is clicked,
     the string "someValue" is presented on port 'infoOutPrt'

     When any string is attached in port 'infoInPrt', this string is presented as text.

-->
<ncl id="yellowRegClick"  xmlns="http://www.ncl.org.br/NCL3.0/EDTVProfile">
  <head>
    <regionBase>
      <region id="backgdReg" left="0%" top="0%" width="100%" height="100%" zIndex="1" />
      <region id="redReg"    left="25%" top="25%" width="25%" height="25%" zIndex="2" />
      <region id="yellowReg" left="50%" top="50%" width="25%" height="25%" zIndex="2" />
    </regionBase>
    <descriptorBase>
      <descriptor id="backgdDsc" region="backgdReg" >
	<descriptorParam name="background" value="gray"/>      	
      </descriptor>
      <descriptor id="yellowDsc" region="yellowReg" focusIndex="1" >
	<descriptorParam name="background" value="yellow"/>      	
      </descriptor>
      <descriptor id="redDsc" region="redReg" focusIndex="1" >
	<descriptorParam name="background" value="red"/>      	
      </descriptor>
    </descriptorBase>
    <connectorBase>
      <causalConnector id="onSelectionSetStart">
        <connectorParam name="var"/>
        <simpleCondition role="onSelection"/>
	<compoundAction modifier="par" >
	  <simpleAction role="set" value="$var" max="unbounded"/>
	  <simpleAction role="start" max="unbounded" />
	</compoundAction>
      </causalConnector>
      <causalConnector id="onEndAttributionStart">
        <simpleCondition role="onEndAttribution"/>
        <simpleAction role="start" max="unbounded" /> 
      </causalConnector> 
    </connectorBase>
  </head>
  <body>
    <port id="backgd" component="backgdMed"/>
    <port id="yellow" component="yellowMed"/>
    <port id="infoInPrt" component="yellowMed" interface="infoInPrt"/>
    <port id="infoOutPrt" component="yellowMed" interface="infoOutPrt"/>

    <media id="backgdMed" descriptor="backgdDsc"/>
    <media id="yellowMed" descriptor="yellowDsc">
    	<property name="infoInPrt"/>
    	<property name="infoOutPrt"/>
    </media>
    <media id="redMed" descriptor="redDsc">
    	<property name="infoRedPrt"/>
    </media>

    <link id="l1" xconnector="onSelectionSetStart">
      <bind role="onSelection" component="yellowMed"/>
      <bind role="start" component="redMed"/>
      <bind role="set" component="yellowMed" interface="infoOutPrt">
      	<bindParam name="var" value="NCLstringYellow"/>
      </bind>
    </link>
    <link id="l2" xconnector="onEndAttributionStart">
      <bind role="start" component="redMed"/>
    </link>
  </body>
<!--
      <bind role="set" component="redMed" interface="infoRedPrt">
      	<bindParam name="var" value="NCLstringRed"/>
      </bind>
-->
</ncl>
