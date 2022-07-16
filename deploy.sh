#!/bin/bash

DEPLOYMENT_ROOT=~/lights

if cmp -s "python/alexa.py" "$DEPLOYMENT_ROOT/python/alexa.py" ; then
   echo "No changes to alexa.py"
   alexa_changed="false"
else
   alexa_changed="true"
fi

if cmp -s "python/update_ngrok.py" "$DEPLOYMENT_ROOT/python/update_ngrok.py" ; then
   echo "No changes to update_ngrok.py"
   ngrok_changed="false"
else
   ngrok_changed="true"
fi

mkdir -p $DEPLOYMENT_ROOT/logs
rm -rf $DEPLOYMENT_ROOT/python

cp -R python $DEPLOYMENT_ROOT/python
cp run.sh $DEPLOYMENT_ROOT

sudo cp systemd/alexa.service /etc/systemd/system/alexa.service
sudo cp systemd/ngrok.service /etc/systemd/system/ngrok.service
sudo systemctl daemon-reload

if [[ "$alexa_changed" == "true" ]]; then
  echo "Restarting alexa service"
  sudo systemctl restart alexa.service
else
  echo "Did not need to restart alexa service"
fi

if [[ "$ngrok_changed" == "true" ]]; then
  echo "Restarting ngrok service"
  sudo systemctl restart ngrok.service
else
  echo "Did not need to restart ngrok service"
fi


if cmp -s "nginx/alexa" "/etc/nginx/sites-available/alexa" ; then
   echo "Did not need to restart nginx"
else
   sudo cp nginx/alexa /etc/nginx/sites-available/alexa
   sudo service nginx restart
   echo "Restarted nginx"
fi

