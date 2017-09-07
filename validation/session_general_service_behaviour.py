
def session_general_service_behaviour_tests(client):

  # test 001
  client.create_session()
  client.close_session()


