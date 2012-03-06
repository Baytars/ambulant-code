<?xml version="1.0" encoding="ISO-8859-1"?>
<ncl id="test82"  xmlns="http://www.ncl.org.br/NCL3.0/EDTVProfile">
<!--
    yellowRegClick.ncl: very simple example to render a yellow rectangle
    on a gray background
--> 
  <head>
    <regionBase>
      <region id="backgdReg" left="0%" top="0%" width="100%" height="100%" zIndex="1" />
      <region id="yellowReg" left="50%" top="50%" width="25%" height="25%" zIndex="2" />
    </regionBase>
    <descriptorBase>
      <descriptor id="backgdDsc" region="backgdReg" >
	<descriptorParam name="background" value="gray"/>      	
      </descriptor>
      <descriptor id="yellowDsc" region="yellowReg"  focusIndex="1" >
	<descriptorParam name="background" value="yellow"/>      	
      </descriptor>
    </descriptorBase>
  </head>
  <body>
    <port id="backgd" component="backgdMed"/>
    <port id="yellow" component="yellowMed"/>

    <media id="backgdMed" descriptor="backgdDsc"/>
    <media id="yellowMed" descriptor="yellowDsc"/>
  </body>
</ncl>
