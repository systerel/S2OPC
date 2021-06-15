from sys import modules
import tests.PubSub.scripts.wait_publisher

class S2OPCTest:
	def wait_publisher(self):
		ret = tests.PubSub.scripts.wait_publisher.wait_publisher(tests.PubSub.scripts.wait_publisher.DEFAULT_URL,tests.PubSub.scripts.wait_publisher.TIMEOUT)
		if not ret:
			raise Exception(f"Cannot connect to publisher at {tests.PubSub.scripts.wait_publisher.DEFAULT_URL}")