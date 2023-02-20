import React, { Component, useState } from 'react';
import { View, StyleSheet, Alert, TouchableOpacity, Text, TouchableHighlight, TextInput} from 'react-native';

//export default class Post extends Component 

const App = () => {
  // async componentDidMount() {
  //   // await this.postData('Post Sample')
  // }
  const [IPadress, setIPadress] = useState('');
  postData = async(str, IPadress) => {
    try {
      let res = await fetch('http://'+IPadress+':8000/', {
        method: 'POST',
        headers: {
          'Host' : '192.168.209.59',
          'Content-Type': 'application/json',
          'Content-Length': 27,
        },
        body: JSON.stringify({
          str,
        }),
      });
      res = await res.json();
      console.log(res)
    } catch (e) {
      console.error(e);
    }
  }
    return (
      <View style={styles.container}>
      <Text style={styles.Title}> Mode d'allumage </Text>
      <TextInput
          value={IPadress}
          onChangeText={(IPadress) => setIPadress(IPadress)}
          placeholder={'the IPadress of server'}
          style={styles.input}
        />
        <TouchableOpacity onPress={() => this.postData(1,IPadress)} style={styles.button}>
          <Text style={styles.button_text}>mode 1</Text>
        </TouchableOpacity>
        <TouchableOpacity onPress={() => this.postData(2,IPadress)} style={styles.button}>
          <Text style={styles.button_text}>mode 2</Text>
        </TouchableOpacity>
        <TouchableOpacity onPress={() => this.postData(3,IPadress)} style={styles.button}>
          <Text style={styles.button_text}>mode 3</Text>
        </TouchableOpacity>
        <TouchableOpacity onPress={() => this.postData(4,IPadress)} style={styles.button}>
          <Text style={styles.button_text}>mode 4</Text>
        </TouchableOpacity>
      </View>
    );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#808383',
    justifyContent: "center",
    alignItems: "center",
  },
  button: {
    //style 3d button for react
    backgroundColor: '#fff',
    borderRadius: 10,
    margin : 10,
    padding: 10,
    width: 200,
    height: 50,
    justifyContent: 'center',
    alignItems: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },

    shadowOpacity: 0.8,
    shadowRadius: 2,
    elevation: 5,
  },
  button_text: {
    color: '#000',
    fontSize: 20,
  }, 
  Title: {
    margin :30,
    fontSize: 30,
    fontWeight: 'bold',
    color: '#000',
  },
  input: {
    width: 230,
    height: 50,
    padding: 10,
    fontSize: 20,
    marginTop: 20,
    marginBottom: 10,
    backgroundColor: '#e8e8e8'
  },
  input_text: {
    // fancy TextInput style
    fontSize: 20,
    color: '#000',
    fontWeight: 'bold',
  }
});

export default App;