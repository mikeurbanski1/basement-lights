#!/bin/bash

DEPLOYMENT_ROOT=~/lights

mkdir $DEPLOYMENT_ROOT
mkdir $DEPLOYMENT_ROOT/logs
rm -rf $DEPLOYMENT_ROOT/python

cp -R python $DEPLOYMENT_ROOT/python
cp run.sh $DEPLOYMENT_ROOT

sudo cp systemd/alexa.service /etc/systemd/system/alexa.service
sudo cp systemd/ngrok.service /etc/systemd/system/ngrok.service
sudo systemctl daemon-reload


if cmp -s "nginx/alexa" "/etc/nginx/sites-available/alexa" ; then
   echo "Did not need to restart nginx"
else
   sudo cp nginx/alexa /etc/nginx/sites-available/alexa
   sudo service nginx restart
   echo "Restarted nginx"
fi

