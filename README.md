# drive-backup
Rough outline of the design:
 - RFID tag in phone case
 - tag triggers a request to HTTP server running on ESP32 (or similar dev board)
 - When the server receives a POST request with the correct credentials, triggers backup routine
 - fetch metadata of full list of files from google drive
 - generate unique hash from the filename, date modified, size, etc. 
 - compare hashed identifiers to those in the database of backed up files
 - tag any files in google drive which do not exist in the local database
 - start upload for tagged files. This should probably be a high concurrency process

## First steps
Let's start with building the intermediate steps, mainly building the list of new files to upload. What we need to do is:
 - figure out how to make api requests in C/C++
 - figure out the shape of the google drive api
 - figure out hashing in C/C++, decide on an appropriate algorithm, we want speed and reliability and nothing else, what's the one Torvalds used for Git?
 - What databases work well with C/C++, I assume all of them, probably something lightweight, SQLLite I suppose?