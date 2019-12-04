# copy required *.so file to into lib folder
sudo mkdir -p /usr/lib/steamapi/
[ -f /usr/lib/libsteam_api.so ] || sudo cp steamworks_sdk/redistributable_bin/linux64/libsteam_api.so /usr/lib/steamapi/
sudo ldconfig
echo "Installation complete"