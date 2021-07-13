from sys import modules

import tests.PubSub.scripts.wait_publisher
import tests.PubSub.validation_tools.pubsub_server_test

class S2OPCTest:
	def wait_publisher(self):
		ret = tests.PubSub.scripts.wait_publisher.wait_publisher(tests.PubSub.scripts.wait_publisher.DEFAULT_URL,tests.PubSub.scripts.wait_publisher.TIMEOUT)
		if not ret:
			raise Exception(f"Cannot connect to publisher at {tests.PubSub.scripts.wait_publisher.DEFAULT_URL}")
	
	def test_static_configuration(self):
		ret = tests.PubSub.validation_tools.pubsub_server_test.testPubSubStaticConf()
		print("Test")