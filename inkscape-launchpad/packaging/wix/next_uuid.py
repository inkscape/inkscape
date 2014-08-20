import uuid

"""
a helper script to simply generate some new UUID
"""

out = open('next_uuid.txt', 'wt')
for i in xrange(10):
	out.write(str(uuid.uuid4()) + '\n')
	
