client = DynamicClient('localhost:31763');
rfsa_service = 'nirfsa_grpc.NiRFSA';

init_response = client.query_struct(rfsa_service, 'Init', 'resource_name', 'VST2_01');
close_response = client.query_struct(rfsa_service, 'Close', 'vi', init_response.vi);

client.close();
