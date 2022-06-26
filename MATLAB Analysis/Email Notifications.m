data = thingSpeakRead([Your-ThingSpeak-Channel-ID], "NumMinutes", 1800);

SMoist = mean(data(:, 4));
Battery = mean(data(:, 6));

apiKey = '[Your ThingSpeak Alerts API Key]';
alertURL = "https://api.thingspeak.com/alerts/send";

options = weboptions("HeaderFields", ["ThingSpeak-Alerts-API-Key", apiKey ]);


alertBodyMoist = sprintf("This email is to remind you soil moisture level of smart farms is below 35 percent.");
alertSubjectMoist = sprintf("Your plant needs water.");


alertBodyInfo = sprintf("This email is to remind you that the battery level of smart farms is below 20 percent.");
alertSubjectInfo = sprintf("The battery level is below 20 percent.");

alertBodyWarn = sprintf("This email is to remind you that the battery level of smart farms is below 10 percent. You better need to charge the battery now.");
alertSubjectWarn = sprintf("The battery level of smart farms is below 10 percent.");

alertBodyCrit = sprintf("This email is to remind you that the battery level of smart farms is below 5 percent. Please recharge the battery as soon as possible.");
alertSubjectCrit = sprintf("The battery level of smart farms is critically low, needs recharge as soon as possible.");


if SMoist <= 50
    try
    webwrite(alertURL, "body", alertBodyMoist, "subject", alertSubjectMoist, options);
    catch someException
    fprintf("Failed to send alert: %s\n", someException.message);
    end
end


if Battery <= 3790
    try
    webwrite(alertURL, "body", alertBodyInfo, "subject", alertSubjectInfo, options);
    catch someException
    fprintf("Failed to send alert: %s\n", someException.message);
    end
elseif Battery <= 3730
    try
    webwrite(alertURL, "body", alertBodyWarn, "subject", alertSubjectWarn, options);
    catch someException
    fprintf("Failed to send alert: %s\n", someException.message);
    end
elseif Battery <= 3500
    try
    webwrite(alertURL, "body", alertBodyCrit, "subject", alertSubjectCrit, options);
    catch someException
    fprintf("Failed to send alert: %s\n", someException.message);
    end
end
