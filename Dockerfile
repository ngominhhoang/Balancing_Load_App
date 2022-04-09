FROM python:3.8

#Set the working directory
WORKDIR /

#copy all the files
COPY . .

#Install the dependencies
RUN apt-get -y update
RUN apt-get update && apt-get install -y python3 python3-pip
RUN pip3 install -r requirements.txt

#Expose the required port
ENV PORT=8000

#Run the command
CMD gunicorn --bind 0.0.0.0:$PORT run:app --timeout 600