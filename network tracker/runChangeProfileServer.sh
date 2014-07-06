while(true)
do
	./socketServer_changeProfile
	fuser -k 1182/tcp	
done
