const AWS = require('aws-sdk');
AWS.config.update({region:"us-east-2"});

exports.handler = async (event, context) => {
    
    // create document client object
    const dc = new AWS.DynamoDB.DocumentClient({region:"us-east-2"});
   
    // create request parameters from event values
    const params = {
        TableName : "kegs",
        KeyConditionExpression: "kegId = :kegId",
        ExpressionAttributeValues: {
            ":kegId" : event.kegId
        }
    };
   
    // attempt to get the data from dynamodb
    try {
        var data = await dc.query(params).promise()
        console.log(data);
        return data.Items
    } catch (err) {
        // if an error occurs, log the error and return
        console.log(err);
        return err;
    }
    
}
