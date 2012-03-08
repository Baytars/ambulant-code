<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
     yellowRegClick.ncl: very simple example to render a yellow rectangle
     on a gray background; when the yellow region is clicked,
     the string "someValue" is presented on port 'infoOut'
-->
<ncl id="yellowRegClick"  xmlns="http://www.ncl.org.br/NCL3.0/EDTVProfile">
  <head>
    <regionBase>
      <region id="backgdReg" left="0%" top="0%" width="100%" height="100%" zIndex="1" />
      <region id="yellowReg" left="50%" top="50%" width="25%" height="25%" zIndex="2" />
    </regionBase>
    <descriptorBase>
      <descriptor id="backgdDsc" region="backgdReg" >
	<descriptorParam name="background" value="gray"/>      	
      </descriptor>
      <descriptor id="yellowDesc" region="yellowReg" focusIndex="1" >
	<descriptorParam name="background" value="yellow"/>      	
      </descriptor>
    </descriptorBase>
    <connectorBase>
      <causalConnector id="onSelectionSetStart">
      <connectorParam name="var"/>
      <simpleCondition role="onSelection"/>
      <simpleAction role="set" value="$var" max="unbounded"/>
     </causalConnector>
    </connectorBase>
  </head>
  <body>
    <port id="backgd" component="backgdMed"/>
    <port id="yellow" component="yellowMed"/>
    <port id="infoOut" component="yellowMed" interface="info"/>

    <media id="yellowMed" descriptor="yellowDesc">
    	<property name="info"/>
    </media>

    <link id="l1" xconnector="onSelectionSetStart">
      <bind role="onSelection" component="yellowMed"/>
      <bind role="set" component="yellowMed" interface="info">
      	<bindParam name="var" value="someValue"/>
      </bind>
    </link>
  </body>
</ncl>
