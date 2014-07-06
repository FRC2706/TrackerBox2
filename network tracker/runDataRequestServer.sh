while(true)
do
	./socketServer_dataRequest
	fuser -k 1182/tcp	
done
