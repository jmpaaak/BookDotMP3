./curl -v -k -s -H "Content-Type: application/json" \
   https://vision.googleapis.com/v1/images:annotate?key=AIzaSyCct00PWxWPoXzilFo8BrgeAKawR9OiRZQ \
    --data-binary @request-target.json -o response.json
