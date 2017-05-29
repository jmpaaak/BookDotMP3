wget -O- --post-data='
{
  "requests": [
    {
      "image": {
        "source": {
          "imageUri":                    "hjpg"
		}
	  },
      "features": [
        {
          "type": "TEXT_DETECTION",
        }
      ]
    }
  ]
}

' --header=Content-Type:application/json "https://vision.googleapis.com/v1/images:annotate?key=AIzaSyCct00PWxWPoXzilFo8BrgeAKawR9OiRZQ" -O response.json
