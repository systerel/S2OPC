from sys import modules

import tests.PubSub.scripts.wait_publisher
import tests.PubSub.validation_tools.pubsub_server_test

import os.path

from robot.api import logger

class S2OPCTest:
	def wait_publisher(self):
		ret = tests.PubSub.scripts.wait_publisher.wait_publisher(tests.PubSub.scripts.wait_publisher.DEFAULT_URL,tests.PubSub.scripts.wait_publisher.TIMEOUT)
		if not ret:
			raise Exception(f"Cannot connect to publisher at {tests.PubSub.scripts.wait_publisher.DEFAULT_URL}")
	
	def test_static_configuration(self):
		tests.PubSub.validation_tools.pubsub_server_test.testPubSubStaticConf(['192.0.2.100', '192.0.2.101'])

		#Checking that there is no 'not ok' line in the log
		with open(os.path.dirname(__file__) + '/../../../pubsub_server_test.tap', 'r') as log:
			log.readline()#Skipping the first line that is just the total number of tests
			lines = log.readlines()
			foundNotOk = False

			for line in lines:
				if line[0] == 'n':#not ok entry
					logger.error(line)
					foundNotOk = True
			log.close()

		if foundNotOk:
			raise Exception("Some tests failed, more informations in the log file 'pubsub_server_test.tap'")
		