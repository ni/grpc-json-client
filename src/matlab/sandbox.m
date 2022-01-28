client = DynamicClient('localhost:31763');

init_request = struct('resource_name', 'VST2_01');
init_response = client.query_struct('nirfsa_grpc.NiRFSA', 'Init', init_request);

close_request = struct('vi', init_response.vi);
close_response = client.query_struct('nirfsa_grpc.NiRFSA', 'Close', close_request);

client.close();