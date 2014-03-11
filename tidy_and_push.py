#!/opt/local/Library/Frameworks/Python.framework/Versions/2.7/Resources/Python.app/Contents/MacOS/Python
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Wavechild670
#
# Tidy and push -- adds headers to source files.
# By Peter Raffensperger 2012
# 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
import sys
import os

from projectinfo import *

def add_file_headers_and_do_search_and_replace():
	CPP_EXTENSIONS = ['.h', '.cpp']
	PYTHON_EXTENSIONS = ['.py']
	EXCEPTIONS = ['tidy_and_push.py', 'projectinfo.py', 'gnuplot_i.h', 'gnuplot_i.cpp', 'getopt_pp.cpp', 'getopt_pp.h',]
	SEARCH_AND_REPLACE = {'__version__ =.*?\n': '__version__ = "' + VERSION + '"\n',
		'turntakingmeasurementtoolsversion =.*?\n': 'turntakingmeasurementtoolsversion = "' + VERSION + '"\n',
	}
	for root, dirs, files in os.walk('.'):
		for f in files:
			fpath = os.path.join(root, f)
			extension = os.path.splitext(f)[1]
			
			def modify(bannerStart, bannerEnd, commentStarter=''):
				print "Modifying", fpath
				ff = open(fpath, 'r')
				contents = ff.read()
				ff.close()
				endOfHeader = contents.rfind(bannerEnd)
				header = bannerStart + '\n' + HEADER
				header = header.replace('%filename%', f)
				header = header.replace('\n', '\n' + commentStarter)
				header = header + '\n' + bannerEnd

				if endOfHeader > 0:
					contents = header + '\n' +  contents[endOfHeader+len(bannerEnd)+1:]
				else:
					contents = header + '\n' + contents
				for key in SEARCH_AND_REPLACE:
					contents = re.sub(key, SEARCH_AND_REPLACE[key], contents)
	
				ff = open(fpath, 'w')
				ff.write(contents)
				ff.close()
			
			if extension in CPP_EXTENSIONS and f not in EXCEPTIONS:
				modify(BANNER_CPP_START, BANNER_CPP_END, commentStarter='* ')
			if extension in PYTHON_EXTENSIONS and f not in EXCEPTIONS:
				modify(BANNER_PYTHON, BANNER_PYTHON, commentStarter='# ')

def run_tests():
	print "Running test suite..."
	execfile('test/testall.py')

def commit():
	print "Enter a commit message"
	msg = raw_input()
	os.system('git commit -am "' + msg + '"')

def push_to_google_code():
	print "Pushing changes to Google code git respository..."
	os.system('git push https://code.google.com/p/wavechild670/ master')

def make_source_distribution():
	print "Making source distribution file..."
	fail
	#os.system('python2.7 setup.py sdist --formats=zip')

def make_pdf_sphinx_documentation():
	print "Making pdf documentation..."
	fail
	#os.system('cd docs; make latexpdf')
	#os.system('cp docs/_build/latex/Turn-TakingMeasurementTools.pdf turntakingmeasurementtools_manual.pdf')

add_file_headers_and_do_search_and_replace()
#run_tests()	
#make_pdf_sphinx_documentation()
commit()
push_to_google_code()
#print "make source distribution? (press ENTER for no, or 'y' then ENTER for yes)"
#answer = raw_input()
#if answer == 'y':
#	make_source_distribution()