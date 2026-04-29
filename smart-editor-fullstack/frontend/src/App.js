import React, { useState, useEffect, useRef } from 'react';
import './App.css';

const API_URL = 'http://localhost:8080';

function App() {
    const [editorState, setEditorState] = useState({ text: '', cursor: 0 });
    const editorRef = useRef(null);

    const fetchData = async () => {
        try {
            const response = await fetch(`${API_URL}/state`);
            const data = await response.json();
            setEditorState(data);
        } catch (error) {
            console.error("Error fetching state:", error);
        }
    };

    useEffect(() => {
        fetchData();
    }, []);

    const handleApiCall = async (endpoint, body) => {
        try {
            const response = await fetch(`${API_URL}/${endpoint}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(body),
            });
            const data = await response.json();
            setEditorState(data);
        } catch (error) {
            console.error(`Error with ${endpoint}:`, error);
        }
    };

    const handleInsert = (char) => {
        handleApiCall('insert', { char });
    };

    const handleBackspace = () => {
        handleApiCall('backspace');
    };

    const handleUndo = () => {
        handleApiCall('undo');
    };

    const handleRedo = () => {
        handleApiCall('redo');
    };

    const handleKeyPress = (e) => {
        e.preventDefault();
        if (e.key.length === 1) {
            handleInsert(e.key);
        } else if (e.key === 'Backspace') {
            handleBackspace();
        }
    };

    const focusEditor = () => {
        editorRef.current.focus();
    };

    return (
        <div className="App">
            <h1>Smart Text Editor</h1>
            <div className="editor-container" onClick={focusEditor}>
                <div 
                    className="editor-area" 
                    onKeyDown={handleKeyPress} 
                    tabIndex={0}
                    ref={editorRef}
                >
                    {editorState.text.split('').map((char, index) => (
                        <span key={index} className={index === editorState.cursor ? 'cursor' : ''}>
                            {char}
                        </span>
                    ))}
                    {editorState.cursor === editorState.text.length && <span className="cursor"></span>}
                </div>
            </div>
            <div className="controls">
                <button onClick={handleUndo}>Undo</button>
                <button onClick={handleRedo}>Redo</button>
            </div>
            <div className="instructions">
                <p>Click on the editor area to start typing.</p>
            </div>
        </div>
    );
}

export default App;
