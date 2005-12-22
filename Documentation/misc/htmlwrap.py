"""Generate HTML wrapper around a SMIL document.

The <layout> section of the SMIL document is parsed, and a number
of variables are created. These variables are subsequently substituted
in an HTML template document to create the resulting HTML document.

Variables defined:
%(url)s			The relative url of the SMIL document
%(title)s		The title of the HTML document (defaults to Ambulant example: url)
%(width)s		The width of the SMIL layout section
%(height)s		The height of the SMIL layout section
%(link)s		An <a> link referring to the SMIL document
%(embed)s		A complete <embed> tag referring to the SMIL document
"""

import sys
import os
import getopt
import urllib
import xml.dom.minidom

SCRIPT=__file__
TEMPLATE=os.path.join(os.path.dirname(SCRIPT), 'htmlwrap_template.html')
DEFAULT_WIDTH="240"
DEFAULT_HEIGHT="270"
LINK='<a href="%(url)s">%(url)s</a>'
EMBED='<embed type="application/smil+xml" src="%(url)s" width="%(width)s" height="%(height)s">'
TITLE='Ambulant Plugin example: %(url)s'

def process(template, input, output, title=TITLE):
	dict = {}
	# Compute relative pathname
	input_path = os.path.normpath(os.path.abspath(input))
	output_path = os.path.normpath(os.path.abspath(output))
	common_path = os.path.commonprefix([input_path, output_path])
	common_path = os.path.dirname(common_path)
	if common_path: common_path = common_path + os.path.sep
	input_path = input_path[len(common_path):]
	output_path = output_path[len(common_path):]
	if os.path.split(output_path)[0]:
		print '%s: cannot generate relative path for this output file' % output
		return False
	input_url = urllib.pathname2url(input_path)

	# Parse the document and obtain width/height
	dom = xml.dom.minidom.parse(input)
	root = dom.documentElement
	if not root or root.nodeName != "smil" :
		print "%s: <smil> element missing, or misplaced" % input
		return False
	rootlayouts = dom.documentElement.getElementsByTagName('root-layout')
	toplayouts = dom.documentElement.getElementsByTagName('topLayout')
	layoutnodes = rootlayouts + toplayouts
	if len(layoutnodes) > 1:
		print "%s: Cannot handle more than one root-layout or topLayout" % input
		return False
	width = DEFAULT_WIDTH
	height = DEFAULT_HEIGHT
	if layoutnodes:
		width = layoutnodes[0].getAttribute('width')
		height = layoutnodes[0].getAttribute('height')
		if width[-2:] == 'px': width = width[:-2]
		if height[-2:] == 'px': height = height[:-2]
	
	# Fill the dictionary
	dict['url'] = input_url
	dict['width'] = width
	dict['height'] = height
	dict['link'] = LINK % dict
	dict['embed'] = EMBED % dict
	dict['title'] = title % dict
	
	# Process the template
	tdata = open(template).read()
	odata = tdata % dict
	
	# Write the output file
	open(output, 'w').write(odata)
	return True
	
def usage(long=False):
	print "Usage: %s [opts] [smilfile ...]" % sys.argv[0]
	print "Options:"
	print "--template htmlfile  Use htmlfile as the template (short: -t)"
	if long:
		print
		print __doc__
	
def main():
	template = TEMPLATE
	try:
		opts, args = getopt.getopt(sys.argv[1:], "th", ["template", "help"])
	except getopt.error:
		usage()
		sys.exit(2)
	for o, a in opts:
		if o in ('-t', '--template'):
			template = a
		if o in ('-h', '--help'):
			usage(True)
			sys.exit(0)
	if not args:
		usage()
		sys.exit(2)
	allok = True
	for input in args:
		output = os.path.splitext(input)[0] + '.html'
		ok = process(template, input, output)
		allok = allok and ok
	if allok:
		sys.exit(0)
	sys.exit(1)
	
if __name__ == '__main__':
	main()
	