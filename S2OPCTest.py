from sys import dont_write_bytecode, modules

import tests.PubSub.scripts.wait_publisher
import tests.PubSub.validation_tools.pubsub_server_test

import os.path
import subprocess

from robot.api import logger

b'/home/adrien/zephyrproject\n'

class S2OPCTest:
	def wait_publisher(self):
		ret = tests.PubSub.scripts.wait_publisher.wait_publisher(tests.PubSub.scripts.wait_publisher.DEFAULT_URL,tests.PubSub.scripts.wait_publisher.TIMEOUT)
		if not ret:
			raise Exception(f"Cannot connect to publisher at {tests.PubSub.scripts.wait_publisher.DEFAULT_URL}")

	def test_static_configuration(self):
		tests.PubSub.validation_tools.pubsub_server_test.testPubSubStaticConf(['192.0.2.100', '192.0.2.101'])
		zephyrProjectPath = '/root/zephyrproject' # Zephyr project in the docket image is here

		if not os.path.exists(zephyrProjectPath + '/pubsub_server_test.tap'):
			# This launchs the find command to search where is the 'zephyrproject' folder if we are not on the docker image.
			# The output of find is storred with the '\n' at the end.
			# If the command found the folder, the newline at the end is removed, otherwise, an exception is raised
			findOutputInByte = subprocess.Popen(['find', '-O3', '/home', '-name', 'zephyrproject', '-print'], stdout=subprocess.PIPE).communicate()[0]
			if len(zephyrProjectPath) == 0:
				raise Exception("Cannot found zephyrproject folder")
			else:
				zephyrProjectPath = findOutputInByte.decode('utf-8').rstrip()

		# Checking that there is no 'not ok' line in the log
		with open(zephyrProjectPath + '/pubsub_server_test.tap', 'r') as log:
			log.readline()#Skipping the first line that is just the total number of tests
			lines = log.readlines()
			foundNotOk = False

			for line in lines:
				if line[0] == 'n':# not ok entry
					logger.error(line)
					foundNotOk = True
			log.close()

		if foundNotOk:
			raise Exception("Some tests failed, more informations in the log file 'pubsub_server_test.tap'")

if __name__ == '__main__':
	print(__file__)