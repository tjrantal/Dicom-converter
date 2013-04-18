import os
import subprocess

def processFolder(source,target,prefix):
	dirList=os.listdir(source)
	for fname in dirList:
		print ('File found '+fname)
		subprocess.call(['save_dicom_mjpeg',source+fname,target+prefix+fname+'.avi'])

#path="C:\\MyTemp\\oma\\Timon\\tyo\\AquaLoading2012\\Piloting"  # insert the path to the directory of interest here 
#target = "\\Converted\\cam1\\"
target = "AVIs\\"
source = "DICOM\\"
prefix = "test_"
processFolder(source,target,prefix)
