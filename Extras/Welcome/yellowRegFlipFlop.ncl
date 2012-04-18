<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
     yellowRegClick.ncl: show yellow region; when clicked present string "someValue"
     on port 'infoOut'
-->
<ncl id="yellowRegClick"  xmlns="http://www.ncl.org.br/NCL3.0/EDTVProfile">
  <head>
    <regionBase>
      <region id="yellowReg" left="50%" top="50%" width="25%" height="25%"/>
    </regionBase>
    <descriptorBase>
      <descriptor id="yellowDesc" region="yellowReg" focusIndex="1">
	    <descriptorParam name="background" value="yellow"/>      	
      </descriptor>
    </descriptorBase>
    <connectorBase>
      <causalConnector id="onSelectionTestSet">
      <connectorParam name="var"/>
      <connectorParam name="value"/>
      <compoundCondition operator="and">
         <assessmentStatement comparator="eq">
            <attributeAssessment role="testValue" eventType="attribution" attributeType="nodeProperty"/>
            <valueAssessment value="$value"/>
         </assessmentStatement>
         <simpleCondition role="onSelection"/>
      </compoundCondition>
      <simpleAction role="set" value="$var" max="unbounded"/>
     </causalConnector>
    </connectorBase>
  </head>
  <body>
    <port id="yellow" component="yellowMed"/>
    <port id="infoOut" component="yellowMed" interface="info"/>

    <media id="yellowMed" descriptor="yellowDesc">
    	<property name="info" value="true"/>
    </media>

    <link id="l1" xconnector="onSelectionTestSet">
      <bind role="onSelection" component="yellowMed"/>
      <bind component="yellowMed" interface="info" role="testValue">
         <bindParam name="value" value="true"/>
      </bind>
      <bind role="set" component="yellowMed" interface="info">
      	<bindParam name="var" value="false"/>
      </bind>
    </link>

    <link id="l2" xconnector="onSelectionTestSet">
      <bind role="onSelection" component="yellowMed"/>
      <bind component="yellowMed" interface="info" role="testValue">
         <bindParam name="value" value="false"/>
      </bind>
      <bind role="set" component="yellowMed" interface="info">
      	<bindParam name="var" value="true"/>
      </bind>
    </link>
  </body>
</ncl>
