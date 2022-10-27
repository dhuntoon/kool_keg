const AWS = require('aws-sdk');
AWS.config.update({region:"us-east-2"});

exports.handler = async (event, context) => {
    
    const dc = new AWS.DynamoDB.DocumentClient({region:"us-east-2"});
    
    const params = {
        TableName : "kegs",
        KeyConditionExpression: "kegId = :kegId",
        ExpressionAttributeValues: {
            ":kegId" : event.kegId
        }
    };
    
    try {
        var data = await dc.query(params).promise()
        console.log(data);
        return data.Items
    } catch (err) {
        console.log(err);
        return err;
    }
    
}
