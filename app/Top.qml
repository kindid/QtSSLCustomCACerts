import QtQuick 2.15

Item {
    function request(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = (function(myxhr) {
            return function() {
                print(myxhr)
                print('myxhr.readyState == ', myxhr.readyState);
                if (myxhr.readyState === 4) callback(myxhr)
            }
        })(xhr);
        xhr.error = (function(myxhr) {
            return function() {
                print('error!', myxhr)
            }
        })(xhr);
        print("getting", url)
        xhr.open('GET', url, true);
        xhr.send('');
    }

    Text {
        id: textThing
        anchors.fill: parent
        font.styleName: 'monospace'
        textFormat: Text.PlainText
        text: 'waiting'
        wrapMode: Text.Wrap
        Component.onCompleted: {
//            request('https://127.0.0.1:6666',
            request('https://fake.com:6666',
                function(o) {
                    // nothing coming back
                    print("callback", o.responseText)
                    for(var p in o) { print(p, ' ==', o[p]);}
                    textThing.text = o.responseText;
                });
        }
    }
}
